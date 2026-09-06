# Suppress SIGPIPE where MSG_NOSIGNAL does not exist

Type: task
Status: resolved
Blocked by: none

## Question

macOS has no `MSG_NOSIGNAL`, so `platform.h` falls back to
`#define MSG_NOSIGNAL 0` and the flag silently becomes a no-op.
`grep -rn "SIGPIPE\|SO_NOSIGPIPE" src/` returns no handler and no socket
option, so the remote unauthenticated DoS that was fixed for Linux — an
unauthenticated client closing its socket between a response being queued and
the next flush kills the daemon — is **live on macOS**, a platform now carrying
published CM19A hardware evidence.

Decide and implement the suppression strategy for platforms lacking
`MSG_NOSIGNAL`: `signal(SIGPIPE, SIG_IGN)` once at startup, `SO_NOSIGPIPE` per
socket, or both. Windows needs neither, so whatever lands must not assume POSIX
signals exist.

Independent of every other ticket and fixable now. Charted rather than fixed ad
hoc so the decision and its evidence are recorded.

## Verification

Reproducible on Linux by forcing the fallback path (compile with
`-DMSG_NOSIGNAL=0`), driving a client that disconnects mid-flush, and confirming
the daemon survives. State plainly whether real macOS was used or only the
forced-fallback proxy.

## Resolution

`signal(SIGPIPE, SIG_IGN)` process-wide, via `sigaction`, in a new
`mochad_ignore_sigpipe()` in `src/net/socket_io.c` -- next to the
`#define MSG_NOSIGNAL 0` fallback it exists to compensate for. `mydaemon()`
calls it before the first listener is created, and the startup log line now
states which way SIGPIPE ended up, so the guarantee is observable at runtime
rather than assumed from the source.

**Why process-wide rather than `SO_NOSIGPIPE` per accepted socket.** Both make
`send()` return `-1`/`EPIPE`, which the callers already handle, so at the send
site they are indistinguishable. The difference is the failure mode of each:

- `SO_NOSIGPIPE` must be applied at every socket that is ever written to. There
  are three `accept()` sites today and nothing forces a fourth listener to
  remember. Missing one silently restores the DoS on exactly one port.
- The usual objection to `SIG_IGN` is that it survives `exec()` and would be
  inherited by a child that legitimately wants to die on a closed pipe. The
  daemon contains no `exec*`, `system` or `popen` -- checked, not assumed -- so
  nothing inherits it. `daemon(0, 0)` forks without exec'ing, and the ignore is
  installed inside `mydaemon()`, which runs in the daemonized child in both
  foreground and background modes.

`SO_NOSIGPIPE` was therefore not added as a belt-and-braces second mechanism:
it is unavailable on Linux, invisible where `SIG_IGN` is already in force, and
its only real benefit -- surviving a future change to the process signal
disposition -- is bought with call sites that can be forgotten.

## Evidence

`tests/unit/test_sigpipe.c`, wired into `scripts/validate/unit-tests.sh`,
proves both halves in forked children:

1. without suppression, a write to a closed peer terminates the child with
   `SIGPIPE` -- the defect is real, not theoretical
2. with `mochad_ignore_sigpipe()`, the same write returns `-1`/`EPIPE`
3. `send_all()` survives the same disconnect through a sender that forces
   `flags = 0`

The macOS platform is emulated by passing `flags = 0` to `send()`, which is
exactly what the `MSG_NOSIGNAL` fallback compiles to. `-DMSG_NOSIGNAL=0` was
deliberately not used: glibc defines the macro as `#define MSG_NOSIGNAL
MSG_NOSIGNAL` over an enumerator, so a command-line override redefines it and
`-Werror` rejects the build.

The control case must run before any call to `mochad_ignore_sigpipe()`, since
a signal disposition is process-wide and the child would otherwise inherit the
ignore and pass for the wrong reason.

**Verified on Linux under AddressSanitizer and UBSan, through the
forced-fallback proxy. Not verified on real macOS hardware.** The forced
fallback reproduces the platform's `send()` flags exactly; it does not
reproduce a Darwin kernel.

Landed on branch `fix/sigpipe-suppression`, commit `77291e1`.

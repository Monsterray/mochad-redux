# Suppress SIGPIPE where MSG_NOSIGNAL does not exist

Type: task
Status: open
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

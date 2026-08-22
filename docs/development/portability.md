# Portability

`mochad-redux` is POSIX software. It builds and runs on Linux, and is expected
to build on macOS and the BSDs. It does not build on Windows.

This document records what is actually portable, what is not, and what a port
would involve. All counts below were measured against the tree, not estimated.

## What is already portable

The protocol and device layers are plain C11 with no OS dependencies:

- `src/x10/` — encode, decode, event formatting, device state
- `src/usb/` — endpoint selection and controller writes
- `src/net/diagnostics.c`, `src/net/transport_evidence.c` — diagnostics
- `src/config/config.c` — configuration parsing

`libusb-1.0` is itself cross-platform and works on Windows and macOS, so USB
access is **not** the obstacle. This is worth stating plainly because the
opposite is often assumed.

## Where the OS dependency lives

Every OS-dependent include and feature probe is collected in
[`src/core/platform.h`](../../src/core/platform.h). Two headers keep a narrow,
declaration-only include of their own — `src/config/config.h` needs the
`LOG_*` severity constants and `src/net/socket_io.h` needs `ssize_t` — so the
portability surface is three files rather than the seven it was before.

| Dependency | Used for | Measured usage |
| --- | --- | --- |
| `syslog.h` | operational logging | 113 `syslog()` calls in `src/core/mochad.c` (48 single-line, 65 spanning lines) |
| `poll.h` | readiness polling | `struct pollfd` throughout the main loop, plus the libusb-owned descriptor set |
| `daemon()` | backgrounding | 2 call sites in `src/core/mochad.c` |
| BSD sockets | TCP listeners | `socket` 1, `bind` 2, `listen` 2, `accept` 4, `setsockopt` 3, `fcntl` 2, `ioctl` 1, `close` 12, `read` 1, `write` 1 |
| `signal.h` | SIGINT/SIGTERM/SIGQUIT | `sigaction` in `mydaemon()` |

## macOS

The smaller gap. All three named blockers exist natively — `syslog`, `poll`,
and `daemon()` are all present — so no shim layer is required. Two known
differences to expect:

- **`MSG_NOSIGNAL` does not exist.** `platform.h` falls back to `0`, which
  keeps `send()` valid but does not suppress `SIGPIPE`. macOS uses the
  `SO_NOSIGPIPE` socket option instead, so a macOS build should set that per
  socket, or ignore `SIGPIPE` process-wide.
- **`daemon()` is deprecated** and warns. It still functions; a launchd-style
  service would be the idiomatic alternative.

macOS is untested rather than unsupported by design. Nobody has built it.

## Windows

The larger gap. Windows lacks all three POSIX facilities and separates socket
handles from file descriptors, so the following would need implementing behind
`platform.h`:

| POSIX | Windows equivalent |
| --- | --- |
| `syslog()` / `openlog()` | Event Log, or a file/stderr sink |
| `poll()` | `WSAPoll()` — close enough in shape to alias |
| `daemon()` | a Windows service, or run in the foreground |
| `close()` on a socket | `closesocket()` |
| `read()` / `write()` on a socket | `recv()` / `send()` |
| `fcntl(O_NONBLOCK)` | `ioctlsocket(FIONBIO)` |
| `ioctl(FIONREAD)` | `ioctlsocket(FIONREAD)` |
| — | `WSAStartup()` / `WSACleanup()` at start and exit |

Note that the 113 `syslog()` call sites do **not** need editing. The
established approach is to supply the POSIX API on the platform that lacks it,
so the call sites stay as they are and only `platform.h` grows a Windows
branch. That also avoids touching the 65 calls that span lines, where a
mechanical rewrite would be error-prone.

Until that exists, `platform.h` fails the build with one clear message rather
than a cascade of missing-header errors.

## Current status

- Platform surface centralized: **done**
- POSIX behaviour unchanged: verified by the strict libusb-free compile
  (`-Wall -Wextra -Werror` with ASan and UBSan), the autotools build with no
  compiler diagnostics, and the full unit suite
- macOS build: **untested**
- Windows implementation: **not started**

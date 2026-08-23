# Windows build for mochad-redux

Label: `wayfinder:map`

## Destination

`mochad-redux` compiles on Windows: `platform.h` grows a Windows branch that
supplies the POSIX service layer the daemon depends on, so the tree builds with
a Windows toolchain. The X10, USB, encode/decode and diagnostics code is
already portable C11 and is not expected to change.

Reaching the destination does **not** claim a supported, hardware-validated
Windows platform. It claims a build. Support level follows evidence, per
`docs/installation/supported-platforms.md`.

## Notes

**This effort carries execution.** Wayfinder defaults to producing decisions;
here the destination is a build, so implementation tickets are in scope and
marked `task`. Decision tickets still come first where a choice is genuinely
open.

Domain: `mochad-redux` is a POSIX daemon for X10 CM15A/CM19A USB controllers.
It exposes three TCP listeners and talks to the controller through
`libusb-1.0`. `libusb` is itself cross-platform, so USB access is not the
obstacle; the surrounding service layer is.

Measured dependency surface (`develop`, commit `88c2e79`):

| Dependency | Usage |
| --- | --- |
| `syslog` | 121 calls, plus `openlog`/`closelog`/`setlogmask` once each; 8 `LOG_*` severities; `LOG_PID`, `LOG_PERROR`, `LOG_LOCAL5`, `LOG_UPTO` |
| `poll` | `struct pollfd` through the main loop plus the libusb-owned descriptor set |
| `daemon()` | 2 call sites |
| BSD sockets | `socket` 1, `bind` 2, `listen` 2, `accept` 4, `setsockopt` 3, `fcntl` 2, `ioctl` 1, `close` 12, `read` 1, `write` 1 |
| `signal` | `sigaction` for SIGINT/SIGTERM/SIGQUIT |

The 121 `syslog()` call sites are **not** to be renamed. The approach is to
supply the POSIX API where the platform lacks it, so call sites stay untouched.
65 of them span lines, which makes a mechanical rewrite the risky option.

Standing constraint: **there is no Windows C toolchain on the development
machine.** Anything claimed as building on Windows must say how it was
verified. Do not report unverified compilation as success.

Relevant existing work: `src/core/platform.h` centralizes the OS-dependent
surface (branch `feature/portability-seam`, not yet merged);
`docs/development/portability.md` records the assessment.

## Decisions so far

<!-- one line per resolved ticket -->

## Not yet specified

- **Service installation and lifecycle.** If a Windows service is chosen, how
  it is registered, started, and removed, and whether that belongs in
  `mochad-redux-setup` or a separate Windows tool. Hangs on ticket 04.
- **Windows packaging and distribution.** Whether anything is shipped at all,
  and in what form. Hangs on 02 and 04.
- **Config and path conventions.** `/etc/mochad-redux/mochad.conf` and
  `/var/lib` have no Windows equivalent; the config precedence chain may need a
  platform-specific default. Hangs on 02.
- **CI coverage.** Whether a Windows job joins the workflows, and what it can
  honestly assert. Hangs on 02 and 11.
- **Hardware reality on Windows.** Whether a CM19A is actually reachable
  through the chosen libusb backend, and what driver the user must install.
  Sharpens once 03 resolves.

## Out of scope

- **Windows hardware validation and a support-level upgrade.** The destination
  is a build. Claiming Windows as supported needs recorded hardware evidence on
  a real Windows host, which is a separate effort.
- **Docker Desktop as a substitute.** Running the Linux container on Windows
  already works and is documented; it does not need this map.

# Milestone 1: Maintenance Foundation

Milestone 1 should make the fork dependable to build, inspect, and maintain
before protocol or behavioral changes are introduced.

## Goals

- Confirm the upstream relationship and keep history rebase-friendly.
- Keep `develop` as the ongoing integration branch.
- Add CI that verifies the autotools build on a clean Linux runner.
- Add a libusb-free compile check for protocol/state/encoding source files.
- Add optional clang-tidy, cppcheck, ASan, and UBSan compile-check modes.
- Fix concrete safety and ownership issues before broad refactoring.
- Document the local build path and required dependencies.
- Improve diagnostics and configuration only where behavior remains compatible.

## Non-goals

- No protocol changes.
- No topic, command, or X10 behavior changes.
- No large rewrites.
- No vendoring of external dependencies.
- No broad USB/TCP/protocol/state separation until safety fixes are stable.

## Acceptance checks

- `git fetch upstream --tags` succeeds.
- `develop` can be compared directly with `upstream/master`.
- `sh tools/compile_without_libusb.sh --strict` passes without libusb installed.
- Ubuntu LTS, Ubuntu Latest, and Debian Stable pass `./autogen.sh`,
  `./configure`, and `make` in CI.
- Raspberry Pi ARM cross compile passes for libusb-free source files in CI.
- Safety boundaries for USB writes, command remainders, endpoint discovery,
  per-client parser state, socket ownership, and debug log clarity are
  documented.
- Local build failures clearly identify missing host dependencies.
- Any diagnostics or configuration changes are documented before release.

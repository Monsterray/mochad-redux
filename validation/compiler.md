# Compiler and Static-Analysis Validation

Compiler validation proves the source still builds across the supported
maintenance targets. Static analysis helps catch portability and safety issues
before hardware testing.

## Required Local Checks

Run these before release candidates and before broad maintenance merges:

```sh
scripts/validate/clean-build-test.sh --libusb-free-only
scripts/validate/strict-libusb-free-compile.sh
scripts/validate/libusb-stub-syntax-check.sh
scripts/validate/unit-tests.sh
scripts/validate/tcp-diagnostics-smoke-test.sh
sh tools/compile_without_libusb.sh --cppcheck
git diff --check
```

Use `scripts/validate/clean-build-test.sh` for release candidates on a Linux
host with libusb headers. It removes ignored build outputs before running the
strict libusb-free compile, full Autotools build, and native smoke test.

`libusb-stub-syntax-check.sh` is for development machines without libusb
headers. It compiles `src/core/mochad.c` with the checked-in header under
`tools/stubs/libusb-1.0/` so maintainers can catch ordinary compile mistakes in
USB-facing code. This is build assistance only; release evidence still needs a
real Linux/libusb build.

`unit-tests.sh` runs strict warning, ASan, and UBSan execution checks for the
configuration, diagnostics, and socket-output helpers.
`tcp-diagnostics-smoke-test.sh` validates that the diagnostic JSON builders can
produce single-line JSON over a real loopback TCP socket.

## Full Linux Build

On a Linux host with libusb development headers installed:

```sh
scripts/validate/full-libusb-build.sh
```

Record:

- Distribution and version.
- Architecture.
- Compiler and version.
- libusb package version, if known.
- Result and warnings.

## Optional Analyzer Checks

Run when available:

```sh
sh tools/compile_without_libusb.sh --clang-tidy
sh tools/compile_without_libusb.sh --clang-format-check
sh tools/compile_without_libusb.sh --cppcheck-style
```

These checks are useful for maintainers, but they should not block emergency
bug fixes unless the reported issue is safety-relevant.

## CI Evidence

For releases, link the GitHub Actions run that covers:

- Ubuntu LTS.
- Ubuntu Latest.
- Debian Stable.
- Raspberry Pi ARM cross compile for libusb-free sources.

If a CI target is unavailable, document why and whether the release is still
acceptable.

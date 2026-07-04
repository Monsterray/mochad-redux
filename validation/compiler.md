# Compiler and Static-Analysis Validation

Compiler validation proves the source still builds across the supported
maintenance targets. Static analysis helps catch portability and safety issues
before hardware testing.

## Required Local Checks

Run these before release candidates and before broad maintenance merges:

```sh
sh tools/compile_without_libusb.sh --strict --asan --ubsan
sh tools/compile_without_libusb.sh --cppcheck
git diff --check
```

## Full Linux Build

On a Linux host with libusb development headers installed:

```sh
./autogen.sh
./configure
make
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

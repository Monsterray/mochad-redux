# Contributing

Thank you for helping maintain `mochad-redux`.

This project is in a stewardship phase. The priority is confidence,
compatibility, validation evidence, and maintainable releases rather than rapid
feature growth.

## Development Principles

- Preserve existing `mochad` protocol behavior.
- Keep changes small and reviewable.
- Prefer diagnostics, safety, tests, and documentation over new features.
- Avoid broad USB, TCP, protocol, or state rewrites without evidence and review.
- Keep generated artifacts out of unrelated changes.

## Before Opening a Pull Request

Run:

```sh
sh tools/compile_without_libusb.sh --strict --asan --ubsan
sh tools/compile_without_libusb.sh --cppcheck
git diff --check
```

When possible, also run a full Linux build:

```sh
./autogen.sh
./configure
make
```

For release PRs, complete [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) and
record validation evidence using
[validation/release-evidence-template.md](validation/release-evidence-template.md).

## Hardware Reports

Hardware validation is valuable even when no code changes are involved. Use the
hardware validation issue template or [validation/hardware.md](validation/hardware.md)
to report CM19A or CM15A results.

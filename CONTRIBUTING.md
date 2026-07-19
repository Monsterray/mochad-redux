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
scripts/validate/clang-format.sh
scripts/validate/shellcheck.sh
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

Changes to installation, templates, or native setup must also run:

```sh
scripts/validate/staged-install-contract.sh
scripts/validate/native-setup-tool.sh
```

Do not add host-mutating behavior to `make install`. Native systemd, udev, and
account changes belong in the explicit `mochad-redux-setup` administration
tool.

## Formatting and Shell Scripts

Maintained C and header files follow the checked-in [`.clang-format`](.clang-format)
policy: four spaces, attached braces, 100-column limit, and right-aligned
pointers. Run `scripts/format-c.sh` before committing C formatting changes.
The development libusb stub is intentionally excluded.

All tracked shell scripts are checked with ShellCheck at warning severity.
Fix shell defects rather than adding broad suppressions. The initial finding
disposition is recorded in [docs/static-analysis-audit.md](docs/static-analysis-audit.md).

## Hardware Reports

Hardware validation is valuable even when no code changes are involved. Use the
hardware validation issue template or [validation/hardware.md](validation/hardware.md)
to report CM19A or CM15A results.

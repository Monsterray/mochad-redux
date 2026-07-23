# Validation Framework

`mochad-redux` releases should be backed by evidence. A release is stronger
when future maintainers can see what was built, where it was tested, which
hardware was used, and which limitations were known at release time.

This directory defines the lightweight validation framework for release
evidence. It does not add product features and it does not replace CI. It gives
maintainers a consistent place to record compiler, static-analysis, regression,
Docker, and hardware validation results.

## Evidence Types

Each release should include:

- Compiler validation: native Linux builds, libusb-free object builds, and
  cross-compile checks.
- Static-analysis validation: cppcheck and clang-tidy where available.
- Runtime validation: startup, listener, USB, client, and shutdown behavior.
- Hardware validation: CM19A and, when available, CM15A results.
- Regression validation: commands and logs that show compatibility-sensitive
  behavior still works.

## Release Evidence

For each release candidate, copy
[release-evidence-template.md](release-evidence-template.md) into a release
tracking issue, pull request, or `validation/releases/` record and fill it in.

The preferred filename for committed release evidence is:

```text
validation/releases/vX.Y.Z.md
```

Committed release evidence should not include private hostnames, private paths,
MQTT credentials, USB serial details, or unrelated logs.

## Validation Scripts

Release-oriented helper scripts live under `scripts/validate/`:

```sh
scripts/validate/clean-build-test.sh
scripts/validate/strict-libusb-free-compile.sh
scripts/validate/libusb-stub-syntax-check.sh
scripts/validate/full-libusb-build.sh
scripts/validate/docker-build.sh
scripts/validate/native-smoke-test.sh
scripts/validate/staged-install-contract.sh
scripts/validate/native-setup-tool.sh
scripts/validate/unit-tests.sh
scripts/validate/tcp-diagnostics-smoke-test.sh
scripts/validate/docker-smoke-test.sh
scripts/validate/cm19a-hardware-validation.sh
```

`clean-build-test.sh` removes ignored build artifacts with `git clean -fdX`
before running validation. Use it when release evidence needs to prove the
build did not depend on stale Autotools files, object files, or previous
binary outputs. In environments without libusb development headers, run:

```sh
scripts/validate/clean-build-test.sh --libusb-free-only
scripts/validate/libusb-stub-syntax-check.sh
```

The stub syntax check compiles `src/core/mochad.c` against a deliberately incomplete
development-only libusb header. It catches normal compiler errors on machines
without libusb headers, but it is not runtime evidence and does not replace the
full Linux/libusb build.

`unit-tests.sh` executes sanitizer-backed tests for socket writes,
configuration precedence/validation, and diagnostic JSON builders.
`tcp-diagnostics-smoke-test.sh` sends diagnostic JSON over a loopback TCP
socket and validates each response line as JSON. In restricted sandboxes where
loopback listeners are blocked, record that limitation and rely on CI for the
TCP evidence.

The scripts print clear console output suitable for pasting into release
evidence. Docker scripts default to the sibling `../mochad-docker` project and
can be pointed elsewhere with `MOCHAD_DOCKER_DIR`.

`staged-install-contract.sh` proves `make DESTDIR=... install` remains
packaging-safe. `native-setup-tool.sh` validates setup behavior against a
temporary fake filesystem root; neither script changes the host or opens USB.

## Hardware Evidence

Hardware validation should follow [hardware.md](hardware.md) and
[docs/hardware-validation.md](../docs/hardware-validation.md). CM19A validation
is required before a release can claim production confidence. CM15A validation
is strongly preferred and should be called out clearly when unavailable.

## Stewardship Rule

When in doubt, prefer documenting what is known over implying confidence. A
release note that says "CM15A not yet validated for this release" is better
than a vague claim of support.

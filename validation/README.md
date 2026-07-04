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
scripts/validate/strict-libusb-free-compile.sh
scripts/validate/full-libusb-build.sh
scripts/validate/docker-build.sh
scripts/validate/native-smoke-test.sh
scripts/validate/docker-smoke-test.sh
scripts/validate/cm19a-hardware-validation.sh
```

The scripts print clear console output suitable for pasting into release
evidence. Docker scripts default to the sibling `../mochad-docker` project and
can be pointed elsewhere with `MOCHAD_DOCKER_DIR`.

## Hardware Evidence

Hardware validation should follow [hardware.md](hardware.md) and
[docs/hardware-validation.md](../docs/hardware-validation.md). CM19A validation
is required before a release can claim production confidence. CM15A validation
is strongly preferred and should be called out clearly when unavailable.

## Stewardship Rule

When in doubt, prefer documenting what is known over implying confidence. A
release note that says "CM15A not yet validated for this release" is better
than a vague claim of support.

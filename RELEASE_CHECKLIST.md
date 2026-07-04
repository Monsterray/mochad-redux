# Release Checklist

Use this checklist for release candidates and final releases. The goal is to
ship releases with evidence, not just a changelog.

## Scope

- [ ] Confirm the release branch is `master`.
- [ ] Confirm development changes were merged from `develop`.
- [ ] Confirm the release scope is maintenance, diagnostics, compatibility, or
  documented behavior.
- [ ] Confirm no broad protocol or architecture change slipped in without
  explicit review.

## Source Hygiene

- [ ] Review `git status --short`.
- [ ] Review generated artifacts against
  [docs/generated-artifacts.md](docs/generated-artifacts.md).
- [ ] Confirm no local build outputs, credentials, private paths, or hardware
  serial details are committed.
- [ ] Confirm `README.md`, `LICENSE.md`, `MAINTAINING.md`, `ROADMAP.md`, and
  validation docs are current.

## Build and Analysis

- [ ] `sh tools/compile_without_libusb.sh --strict --asan --ubsan`
- [ ] `sh tools/compile_without_libusb.sh --cppcheck`
- [ ] `git diff --check`
- [ ] `./autogen.sh`
- [ ] `./configure`
- [ ] `make`
- [ ] Optional: `sh tools/compile_without_libusb.sh --clang-tidy`
- [ ] Optional: `sh tools/compile_without_libusb.sh --clang-format-check`

## CI

- [ ] Ubuntu LTS passed.
- [ ] Ubuntu Latest passed.
- [ ] Debian Stable passed.
- [ ] Raspberry Pi ARM cross compile passed.
- [ ] CI run URL is recorded in release evidence.

## Runtime and Hardware

- [ ] Native foreground startup logs recorded.
- [ ] Native foreground TCP connection test recorded.
- [ ] Native foreground shutdown logs recorded.
- [ ] IPv4 default bind validated.
- [ ] Explicit IPv6 bind validated or limitation documented.
- [ ] CM19A hardware validation linked.
- [ ] CM15A hardware validation linked or explicitly unavailable.
- [ ] Docker validation linked when Docker packaging is part of the release.

## Release Evidence

- [ ] Fill out `validation/release-evidence-template.md`.
- [ ] Link hardware validation issue(s).
- [ ] Link CI run(s).
- [ ] Record known limitations.
- [ ] Record validation commands and results.

## Publish

- [ ] Update release notes with evidence links.
- [ ] Tag release from `master`.
- [ ] Confirm tag points to the reviewed release commit.
- [ ] Confirm post-release roadmap status is still accurate.

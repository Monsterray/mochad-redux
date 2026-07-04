# PROJECT.md

# mochad-redux

`mochad-redux` is a maintained modernization fork of mochad for X10 CM15A/CM19A controllers.

The goal is to preserve mochad compatibility while improving build reliability, diagnostics, Docker usability, and long-term maintainability.

`v0.3.0` is the current baseline. Active integration work targets `v0.4.0`,
focused on hardware validation, issue templates, runtime diagnostics, and small
testable seams.

The engineering principles for this maintained line are documented in
[DESIGN.md](DESIGN.md).

## Project Goals

* Keep existing mochad behavior compatible.
* Build cleanly on modern Linux distributions.
* Support Docker-first deployment.
* Improve diagnostics without breaking old clients.
* Keep the codebase small and understandable.
* Add tests before behavior changes.
* Prefer safe maintenance over rewrites.

## Supported Targets

Primary:

* Linux
* Docker on Linux
* Raspberry Pi OS / Debian / Ubuntu
* CM19A
* CM15A

Secondary:

* macOS compile-only checks where practical
* CI object-build checks without libusb

## Non-Goals

* Full rewrite
* Native Windows support
* Replacing Home Assistant/MQTT bridges
* Large protocol redesign
* Breaking existing mochad TCP clients

## Development Rules

* Preserve compatibility first.
* Avoid broad refactors without tests.
* Keep USB-specific code isolated.
* Keep protocol parsing/encoding testable without hardware.
* Use clear logs and meaningful exit errors.
* Do not require libusb for every test.
* Keep hardware validation repeatable with documented manual checks.

## Release Branches

* `master` is the stable release branch.
* `develop` is the integration branch.
* Release pull requests flow from `develop` into `master`.
* Release tags are created from `master`.

# ROADMAP.md

# Roadmap

## v0.1.0 - Baseline Fork

Goal: preserve working mochad behavior while making the project buildable, documented, and testable.

* [ ] Confirm upstream baseline
* [ ] Verify CM19A works
* [ ] Verify CM15A if hardware is available
* [ ] Add Docker image
* [ ] Add README
* [ ] Add license
* [ ] Add GitHub Actions
* [ ] Add libusb-free compile check
* [ ] Add full libusb build check
* [ ] Document safety boundaries for USB writes, command parsing, endpoint
  discovery, socket ownership, and debug logs

## v0.2.0 - Build and Code Hygiene

Goal: modernize safely without behavior changes.

* [ ] Fix compiler warnings
* [ ] Keep concrete safety fixes reviewable and behavior-preserving
* [ ] Separate USB-specific and non-USB code where practical
* [ ] Improve Makefile/autotools reliability
* [ ] Add basic unit/compile tests
* [ ] Add CI matrix for supported Linux targets
* [ ] Document supported platforms

## v0.3.0 - Docker and Runtime Configuration

Goal: make Docker deployment easy and predictable.

* [ ] Environment-variable based entrypoint
* [ ] Document USB passthrough
* [ ] Configurable foreground/raw mode
* [ ] Investigate configurable bind address
* [ ] Investigate configurable TCP port
* [ ] Improve container logs

## v0.4.0 - Diagnostics

Goal: make troubleshooting easier.

* [ ] Better startup logs
* [ ] Better USB error messages
* [ ] Better client connection logs
* [ ] Optional `hello` command
* [ ] Optional `features` command
* [ ] Optional `health` command

## v1.0.0 - Stable Maintained Release

Goal: stable public release suitable for everyday use.

* [ ] Stable Docker image
* [ ] Hardware-tested CM19A path
* [ ] Hardware-tested CM15A path if possible
* [ ] CI passing
* [ ] Documentation complete
* [ ] No known critical bugs

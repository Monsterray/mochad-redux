# ROADMAP.md

# Roadmap

## v0.1.0 - Baseline Fork

Goal: preserve working mochad behavior while making the project buildable, documented, and testable.

Status: complete except hardware validation.

The Docker image and compose packaging are completed in the companion
`mochad-docker` project.

* [x] Confirm upstream baseline
* [ ] Verify CM19A works
* [ ] Verify CM15A if hardware is available
* [x] Add Docker image
* [x] Add README
* [x] Add license
* [x] Add GitHub Actions
* [x] Add libusb-free compile check
* [x] Add full libusb build check
* [x] Document safety boundaries for USB writes, command parsing, endpoint
  discovery, socket ownership, and debug logs

## v0.2.0 - Build and Code Hygiene

Goal: modernize safely without behavior changes.

Status: complete.

* [x] Fix compiler warnings
* [x] Keep concrete safety fixes reviewable and behavior-preserving
* [x] Separate USB-specific and non-USB code where practical
* [x] Improve Makefile/autotools reliability
* [x] Add basic unit/compile tests
* [x] Add CI matrix for supported Linux targets
* [x] Document supported platforms

## v0.3.0 - Docker and Runtime Configuration

Goal: make Docker deployment easy and predictable.

Status: baseline release.

The Docker packaging work is completed in the companion `mochad-docker`
project. The daemon-side runtime flags are completed in `mochad-redux`.

* [x] Environment-variable based entrypoint
* [x] Document USB passthrough
* [x] Configurable foreground/raw mode
* [x] Investigate configurable bind address
* [x] Investigate configurable TCP port
* [x] Improve container logs

## v0.4.x - Runtime Hardening and Observability

Goal: make the daemon self-explanatory through clear, consistent logging and
diagnostics while keeping changes small, testable, and compatible with existing
clients.

Runtime visibility:

* [x] Make startup logs show version, foreground/background mode, raw mode,
  bind address, ports, listener family, and dual-stack status.
* [x] Make USB initialization logs explain device discovery, permissions,
  kernel-driver conflicts, Docker passthrough issues, endpoints, and transfer
  startup.
* [x] Make TCP listener logs show each listener name, address family, bind
  address, port, and bind/listen failures.
* [x] Make client lifecycle logs show accepted, rejected, disconnected, and
  command-received events with client IDs where practical.
* [x] Make shutdown logs explain signal-driven exits, USB cleanup, and abnormal
  exits clearly.

Validation:

* [ ] Validate startup and RF receive behavior with real CM19A hardware.
* [ ] Validate CM15A behavior if hardware is available.
* [x] Add or refine GitHub issue templates for bug reports and hardware
  validation.
* [x] Add small testable seams around diagnostics where they reduce risk.

Quality gates:

* [x] Keep sanitizer compile checks passing.
* [x] Keep cppcheck and clang-tidy guidance current.
* [x] Document sanitizer/static-analysis expectations for maintainers.
* [x] Document Linux, Docker, Raspberry Pi, IPv4, and explicit IPv6
  compatibility expectations.

Avoid broad USB, TCP, protocol, or state refactors in this milestone. Keep new
diagnostic protocol commands additive, single-line JSON, and compatible with
existing clients.

## Future - Generic JSON API

Goal: add a modern local push API without changing existing listener behavior
or encoding integration-specific concepts into the daemon.

Status: design only. See [docs/json-api.md](docs/json-api.md).

* [x] Introduce a normalized internal `mochad_event_t`.
* [ ] Route legacy text, XMLSocket, OpenRemote, and JSON output through event
  formatters.
* [ ] Add persistent `instance_id` and per-start `session_id`.
* [ ] Add an optional JSON-RPC 2.0 listener on port `1102`, disabled by
  default while experimental.
* [ ] Support UTF-8 JSON object per newline framing with request IDs, standard
  JSON-RPC errors, server notifications, and protocol negotiation.
* [ ] Protect the daemon event loop with bounded per-client output queues.
* [ ] Implement read-only `handshake`, `health`, `capabilities`, `config`,
  `state.snapshot`, and atomic `events.subscribe_snapshot` before command
  methods.
* [ ] Add `x10.command.send` only after read-only behavior is stable.
* [ ] Keep Home Assistant entity, discovery, MQTT topic, and config-entry
  concepts outside the daemon protocol.

## Future - Python Client and Integration Migration

Goal: consume the generic JSON API from higher-level integrations while keeping
those integration concerns outside `mochad-redux`.

* [ ] Add `MOCHAD_PROTOCOL=auto|json|legacy` to `mochad-mqtt-bridge` after the
  daemon JSON API is usable.
* [ ] Build a separate async `aiomochad` PyPI library for JSON API clients.
* [ ] Use `aiomochad` to modernize the Home Assistant `mochad` integration from
  legacy YAML/local polling to config-entry-based local push.

## v1.0.0 - Stable Maintained Release

Goal: stable public release suitable for everyday use.

* [ ] Stable Docker image
* [ ] Hardware-tested CM19A path
* [ ] Hardware-tested CM15A path if possible
* [ ] CI passing
* [ ] Documentation complete
* [ ] No known critical bugs

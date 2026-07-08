# Changelog

All notable changes to `mochad-redux` should be recorded here.

This project values release evidence over broad release claims. Each release
entry should link to validation evidence when available.

## Unreleased

### Added

- Lightweight validation framework under `validation/`.
- Release checklist for evidence-based releases.
- Supported-platform documentation.
- Generated-artifact policy.
- Backward-compatible TCP diagnostic commands: `hello`, `capabilities`,
  `health`, `clients`, and `version`, each returning single-line JSON.
- Runtime options to independently enable or disable the legacy XML and
  OpenRemote listeners while preserving their default enabled behavior.
- Partial-write-safe socket output helper with unit coverage.
- Testable diagnostic JSON builders and a read-only `config` TCP diagnostic
  command.
- Sanitizer-backed unit tests for configuration, diagnostics, and socket
  output helpers.
- Sanitizer-backed X10 output queue tests covering failed USB submissions.
- JSON-validating loopback TCP diagnostics smoke test.
- Design document for a future generic JSON-RPC API on an optional disabled-by-
  default listener.
- Normalized internal `mochad_event_t` and legacy formatter golden tests for
  compatibility-preserving event output work.

### Changed

- Release stewardship now requires compiler, static-analysis, runtime, and
  hardware validation evidence where practical.
- `statusprintf()` and `sockprintf()` now clamp formatted output to their stack
  buffers before sending.
- Decoded X10 activity now routes through the internal event dispatcher before
  being rendered to existing legacy listener output.
- The maintained redux version string now lives in one source header.
- USB interrupt transfer lifecycle tracking now uses explicit submitted and
  canceling flags instead of treating transfer pointers as activity state.
- X10 output queue handling now preserves queued commands when USB submission
  fails so the queue can retry instead of silently advancing past the command.
- libusb is now owned through an explicit context, with dynamic poll descriptor
  registration instead of a fixed-size USB pollfd slot.
- USB startup diagnostics now include symbolic libusb error names, automatic
  kernel-driver detach attempts, and stricter interrupt endpoint/packet-size
  validation.

## v0.3.0

Baseline maintained release for runtime configuration and Docker-focused
deployment work.

## Historical Upstream Changes

These entries come from the inherited upstream `ChangeLog`. They are preserved
for protocol and compatibility context.

### mochad-0.1.18

- Added support for IPv6.

### mochad-0.1.17

- Added missing `setClock()` support for beaconing CM15A (`0xA5`).

### mochad-0.1.16

- Fixed the "Could not find endpoints" problem.

### mochad-0.1.15

- Added support for newer CM19A controllers built in 2011 or later.

### mochad-0.1.14

- Fixed an OpenRemote too-many-open-sockets problem that caused delayed
  responses to button presses.
- Added `extended_code_1` command for shutters/blinds control commands.

### mochad-0.1.13

- Fixed the `-d` debug command-line option.
- Correctly decoded the SP554A motion sensor when the switch is in the
  home/away position.
- Added exits to potential infinite loops.
- Added message support for DS12A cover-off tamper events. The DS12A looks like
  two DS10As with the cover on.

### mochad-0.1.12

- Added `--raw-data` option from Marc Merlin. Programs such as MisterHouse can
  decode the raw RF data.

### mochad-0.1.11

- Fixed `xdim` extended dim behavior that only worked for unit code 3.

### mochad-0.1.10

- Fixed a major client socket problem where commands from clients stopped
  working.

### mochad-0.1.9

- Added `getstatus` and `getstatussec` commands:
  - `getstatus <HU>`: get device on/off status.
  - `getstatus <HU> xdim`: get device extended-dim status.
  - `getstatusec <RFSECAddr>`: get RF security device status such as DS10A.
- Added port `1101` for OpenRemote 2.0. This is similar to port `1099` but does
  not include unsolicited event messages. OpenRemote polls for device events
  using `getstatus` and `getstatussec`.

### mochad-0.1.8

- Added support for X10 KR15A Big Red Button.
- Allowed `\r` as well as `\n` for line termination.
- Added more support for Flash XMLSocket clients, including cross-domain policy
  request/response for Chumby widgets.
- Added Python/Tk demo GUI application with on/off buttons and a brightness
  slider for LM465 soft-start lamp modules. See `apps/x10-tk.py`.

### mochad-0.1.7

- Added Flash XMLSocket support on port `1100` for Chumby widgets.
- Fixed RF checksum problem when receiving from CM15A for house codes 9-16.

### mochad-0.1.6

- Added RF event filter to reduce duplicate events.
- Skipped extra leading `0x5D` on received RF packets from CM15A.

### mochad-0.1.5

- Fixed `RFCAM` command so it shows the transmit message.
- Fixed RF command that sent the wrong house in some cases. SourceForge issue
  ID: 3197002.

### mochad-0.1.4

- Fixed `RFCAM` command so it takes house code as a parameter. For example,
  `RFCAM C CAMUP` sends a camera pan-up command on house code C. Previously,
  `RFCAM` commands always used house code N.

### mochad-0.1.3

- Changed the method for sending dim/bright commands to match ActiveHome Pro.
  The new method uses 3 bytes instead of 2 and works better with LM465
  soft-start devices.
- Added example client programs in Perl and bash.
- Added `--version` command-line parameter.
- Added support for SP554 motion sensor.

### mochad-0.1.2

- Fixed device name in log messages. CM19A was reported as CM15A.
- Added install option that makes controllers hot-pluggable through udev on
  Ubuntu and hotplug2 on OpenWrt.
- Converted build system to autoconf and automake.
- Updated mochad wiki pages with command/event reference and OpenWrt build
  instructions.

# Source Lineage

This document records the known source lineage for `mochad-redux`. It is a
release-engineering aid, not a replacement for file-level copyright notices.

## Project Lineage

`mochad-redux` is a maintained fork of `mochad`, a TCP daemon for X10 CM15A and
CM19A USB controllers.

Known lineage:

- Original project: `mochad` on SourceForge.
- Original author visible in source headers: Brian Uechi
  `<buasst@gmail.com>`.
- Later maintainer visible in runtime copyright output and Autotools metadata:
  Neil Cherry.
- `mochad-redux` maintenance repository: `Monsterray/mochad-redux`.

The current repository history does not yet contain a complete machine-readable
record of the exact upstream commit or source archive used to initialize this
fork. Until that is reconstructed, release notes should describe the upstream
base as `mochad 0.1.18`, which matches the preserved historical changelog and
the former Autotools package version.

## Runtime Source Files

The core C runtime files are inherited from mochad unless they were added in
`mochad-redux` history. Existing copyright and GPL notices are preserved.

Examples of inherited files include:

- `mochad.c`
- `decode.c`
- `decode.h`
- `encode.c`
- `encode.h`
- `global.c`
- `global.h`
- `x10state.c`
- `x10state.h`
- `x10_write.c`
- `x10_write.h`

Files added or substantially expanded during `mochad-redux` maintenance include
configuration, diagnostics, event-model, validation, and test support. They are
distributed under the repository license unless a file states otherwise.

## Support Files

The `systemd/`, `udev/`, and `hotplug2/` directories contain deployment support
files for Linux systems. Their exact historical source path should continue to
be audited before adding file-level SPDX identifiers.

Current understanding:

- `systemd/mochad.service` is maintained in this fork for native Linux service
  deployment.
- `udev/91-usb-x10-controllers.rules` and
  `udev/91-usb-x10-controllers.rules-systemd` are maintained in this fork for
  CM15A/CM19A USB permissions and optional systemd activation.
- `hotplug2/` contains legacy OpenWrt-era support files retained for historical
  compatibility.

## CGI Files

The CGI examples are legacy support files.

- `cgi/cgi-lib.pl` is a bundled third-party CGI parsing library. Its embedded
  notice identifies it as cgi-lib.pl 2.18 by Steven E. Brenner and gives
  permission terms that are not GPL text.
- Other CGI files carry mochad/Brian Uechi GPL-style notices.

## SPDX Policy

Do not bulk-add SPDX identifiers until each file's provenance is known.

For now:

- Preserve existing file headers.
- Preserve the `cgi-lib.pl` embedded permission notice.
- Treat unannotated legacy support files as provenance-pending.
- Add SPDX identifiers only when the license of that exact file has been
  verified.

## Release Archive Expectations

Source archives should include:

- `COPYING`
- `LICENSE.md`
- `NOTICE`
- `docs/source-lineage.md`

Release evidence should record the exact source commit used to build and test
the archive.

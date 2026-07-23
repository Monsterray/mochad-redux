# Duplicate And Obsolete File Audit

Audit base: `3c387bda4c308f330c7fb825af7de9baf63af69e`

This report identifies overlap and obsolescence candidates. It does not approve
deletion by itself.

## Summary

| Finding | Classification | Proposed disposition |
| --- | --- | --- |
| Exact duplicate tracked files | None | NOT APPLICABLE |
| Tracked generated build artifacts | None | NOT APPLICABLE |
| Applied patch/reject files | None | NOT APPLICABLE |
| Local ignored build outputs | Present | Keep ignored; clean with existing validator |
| Static and templated systemd units | Near duplicate with different authority | Keep template; archive static unit as research |
| Three udev rule variants | Near duplicate with conflicting activation policy | Keep template; archive two static historical rules |
| Legacy applications | Mixed, site-specific, old runtimes | Move reviewed examples to `contrib/`; archive unsafe/obsolete examples |
| CGI application | Obsolete deployment model and provenance-sensitive dependency | Archive unless explicitly adopted as unsupported contrib |
| OpenWrt hotplug2 files | Old but unique platform support | Keep and move to `packaging/openwrt/` |
| Version definitions | Deliberate generated chain | Keep `VERSION` plus validated generated header |
| Validation wrappers | Layered orchestration, not duplicates | Keep; update paths only |
| Completed plans/audits | Historical evidence | Move to `docs/research/` |

## Exact Duplicates

SHA-256 comparison of all 122 tracked files found no identical content hashes.
Similar filenames therefore cannot be removed as byte-for-byte duplicates.

## Generated Artifacts

The checkout contained ignored generated files such as:

- `configure`
- `Makefile`
- `Makefile.in`
- `aclocal.m4`
- `autom4te.cache/`
- `.deps/`
- object files and the local daemon binary

`git ls-files` confirms none is tracked. `.gitignore`,
`docs/generated-artifacts.md`, `scripts/validate/repository-hygiene.sh`, and
`scripts/validate/clean-build-test.sh` provide replacement control. No cleanup
commit is needed for these local artifacts.

`version.h` is different: it is generated from `VERSION` but intentionally
tracked and validated. Its runtime and release consumers make deletion a
release-contract change, not repository cleanup.

## Systemd Units

### Maintained authority

`systemd/mochad.service.in` is rendered by `mochad-redux-setup`. It supports:

- resolved binary prefix;
- configured service user and group;
- configured supplementary USB group;
- the preserved `mochad.service` unit name.

It participates in native setup tests and staged installation.

### Historical static file

`systemd/mochad.service` hardcodes `/usr/local/bin/mochad`, `mochad:mochad`,
and `x10`. It is not installed by `make install` or rendered by the setup tool.
It remains in `EXTRA_DIST` and is described by source-lineage documentation.

Proposed action: `ARCHIVE AS RESEARCH`, not immediate deletion. Move it to
`docs/research/legacy-packaging/systemd/mochad.service` with a note that
`packaging/linux/systemd/mochad.service.in` is authoritative.

Replacement coverage:

- `scripts/validate/native-setup-tool.sh`
- `scripts/validate/staged-install-contract.sh`
- `scripts/validate/source-archive-validation.sh`
- native setup and rollback documentation

Compatibility risk: packagers may have copied the static file manually.
Archiving preserves access while removing ambiguity.

## Udev Rules

### Maintained authority

`udev/91-usb-x10-controllers.rules.in` is rendered with the configured USB
group. It handles both CM15A `0bc7:0001` and CM19A `0bc7:0002` and assigns mode
`0660` without starting a service.

### Historical variants

`udev/91-usb-x10-controllers.rules` hardcodes group `x10`.

`udev/91-usb-x10-controllers.rules-systemd` additionally asks systemd to start
`mochad.service` from a udev add event. That conflicts with the current
documented separation between device permission assignment and explicit
service administration.

Proposed action: `ARCHIVE AS RESEARCH` for both static variants. Preserve their
Git/source lineage and remove them from authoritative packaging paths and
`EXTRA_DIST` only after the archive path is included.

Replacement coverage:

- maintained `.rules.in` template;
- native setup rendering tests;
- checks for both USB product IDs;
- host rollback documentation.

Compatibility risk: external users may refer directly to old paths. Release
notes must call out the relocation.

`udev/README.md` mixes maintained instructions with an older
`99-cm19a.rules`/`blacklist ati_remote` note. Merge current guidance into native
installation documentation and preserve the old note in research. Do not apply
the blacklist automatically.

## Legacy Applications

### Retain as contrib

- `apps/simplemon.pl`: small TCP monitor with a hardcoded sample host.
- `apps/mochamon.pl`: multi-daemon monitor with site-specific addresses.
- `apps/rfsectopl3.pl`: GPL-noticed RFSEC-to-power-line example with TTS and
  site-specific assumptions.
- `apps/bash.sh`: DS10A example using Bash `/dev/tcp`, a fixed address, and
  `flite`.

Move these under `contrib/apps/`, label them unsupported examples, and retain
their existing notices. They are not runtime, packaging, or tests.

### Archive as research

`apps/x10-tk.py` requires Python 2 `Tkinter`, embeds a private-network address,
and states that it has no error recovery. Modern users should not interpret it
as a supported UI. Its command choices remain useful historical context.

`apps/mochad.scr` kills all `mochad` processes, removes a fixed FIFO, starts a
hardcoded binary path, and advises changing udev to run the script. Those
behaviors conflict with current service ownership and should not be offered as
a maintained executable example.

Proposed action: archive both under `docs/research/legacy-apps/` with warnings.
Do not execute them in validation.

## CGI Files

The four CGI files form one coupled historical application:

- `cgi/x10.pl`
- `cgi/netcat.pl`
- `cgi/getsensors.pl`
- `cgi/cgi-lib.pl`

The entrypoint has hardcoded hosts, no modern authentication or authorization
model, and predates current web-security expectations. `cgi-lib.pl` carries a
third-party permission notice that must remain intact and must not be silently
relicensed.

Proposed action: `ARCHIVE AS RESEARCH` for the complete set unless a maintainer
explicitly adopts it as unsupported `contrib/cgi/`. Do not split the set and do
not delete `cgi-lib.pl` while retaining dependents.

Replacement coverage: native TCP clients and the separately maintained MQTT
bridge provide supported integration paths. They do not reproduce the CGI UI,
so the archive retains historical behavior and provenance.

## OpenWrt And Hotplug2

`hotplug2/20-usb-x10` and `hotplug2/mochad` are inherited OpenWrt-era support.
They are old, but they are not duplicates of systemd or udev:

- they target `/etc/hotplug.d` and `/etc/rc.common`;
- they represent the only tracked non-systemd embedded-Linux integration;
- they cover both controller product IDs.

Proposed action: KEEP and MOVE to `packaging/openwrt/`. Any retirement requires
an OpenWrt compatibility decision and evidence. Age alone is insufficient.

## Version Identity

The following values overlap intentionally:

- `VERSION`: editable source of truth;
- `version.h`: generated C constants;
- `configure.ac`: reads `VERSION` for Autotools metadata;
- runtime diagnostics and `--version`: consumers;
- changelog and release evidence: release assertions.

`scripts/release/update-version-files.sh` and
`scripts/validate/version-consistency.sh` make this a validated generation
chain, not uncontrolled duplication. Keep it intact and update only paths.

The separate `mochad 0.1.18` upstream identity must remain distinct from the
maintained semantic version.

## Validation Logic

The compile and validation scripts have overlapping command names but distinct
levels:

- the compiler harness supports strict flags, sanitizers, cppcheck, and
  clang-tidy;
- small validation wrappers select one release contract;
- source-archive validation exports a clean tree and composes compile/install
  checks;
- clean-build validation removes ignored local outputs;
- hardware validation remains interactive.

No exact script duplicates were found. Do not merge them in a path-only PR.
After reorganization, consider a separate maintainability review of shared
path lists if changes reveal repeated failure points.

## Documentation

`docs/static-analysis-cleanup-plan.md` and `docs/milestone-1.md` describe
completed work. They are not obsolete facts, but they are obsolete as current
instructions. Move them under `docs/research/`.

`docs/static-analysis-audit.md` remains useful evidence and should also move
under the research/audit hierarchy.

Do not delete:

- protocol compatibility documentation;
- hardware validation procedures;
- native install/rollback instructions;
- supported-platform policy;
- generated-artifact policy;
- source lineage;
- release evidence.

## Deletion Approval Matrix

| Candidate | Current recommendation | Deletion approval |
| --- | --- | --- |
| Static systemd unit | Archive as research | NOT APPROVED |
| Two static udev rules | Archive as research | NOT APPROVED |
| Mixed udev README | Merge current text, archive historical text | NOT APPROVED |
| Python 2 Tk app | Archive as research | NOT APPROVED |
| Raw FIFO/MisterHouse script | Archive as research | NOT APPROVED |
| CGI application set | Needs maintainer choice: contrib or research archive | NEEDS REVIEW |
| OpenWrt hotplug2 files | Keep | NOT APPLICABLE |
| Generated local build outputs | Ignored local cleanup only | NOT APPLICABLE |

At this audit stage, no tracked file has enough evidence for irreversible
deletion. The cleanup branch should therefore remain empty unless the
maintainer explicitly approves a candidate after reviewing the archive
placements and compatibility notes.

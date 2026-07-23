# Repository File Inventory

Audit base: `3c387bda4c308f330c7fb825af7de9baf63af69e`

Audit date: 2026-07-22

This is a read-only classification of every file tracked at the audit base.
No file was moved or deleted while preparing it.

## Method

The audit used:

- `git ls-files`, file modes, content hashes, and `git log --follow`;
- `Makefile.am`, `configure.ac`, `.gitignore`, and the generated-artifact policy;
- C include references and source lists in build and validation scripts;
- current documentation, CI, installation, release, and validation references;
- comparison with `upstream/master` for inherited support files.

There are 122 tracked files. No two tracked files have the same SHA-256
content hash. No patch, reject, object, binary, generated `configure`, generated
`Makefile.in`, or other forbidden build artifact is tracked. Ignored generated
Autotools and compiler output was present locally but is covered by
`.gitignore` and is not part of this inventory.

Participation codes:

- `B`: build input
- `R`: runtime
- `T`: test
- `P`: packaging or installation
- `Rel`: release
- `H`: hardware validation
- `D`: documentation or contributor workflow

Actions are proposals only. `MOVE` means a mechanical `git mv` with all live
references updated. `ARCHIVE AS RESEARCH` preserves provenance while removing
the file from authoritative packaging or installation paths.

## Root And Repository Metadata

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `.clang-format` | Maintained C formatting policy | Development, maintained | T, D | `scripts/format-c.sh`, `scripts/validate/clang-format.sh`, `CONTRIBUTING.md`, static-analysis docs | KEEP at root; standard tool discovery path. |
| `.clang-format-ignore` | Excludes non-maintained C from formatting | Development, maintained | T, D | Both formatting scripts | KEEP at root; moving it would weaken automatic discovery. |
| `.github/ISSUE_TEMPLATE/beta_tester.yml` | Structured beta report form | GitHub, maintained | D | GitHub path convention | KEEP; implicitly loaded by GitHub. |
| `.github/ISSUE_TEMPLATE/bug_report.yml` | Structured defect report form | GitHub, maintained | D | GitHub path convention | KEEP. |
| `.github/ISSUE_TEMPLATE/config.yml` | Issue-template configuration | GitHub, maintained | D | GitHub path convention | KEEP. |
| `.github/ISSUE_TEMPLATE/hardware_validation.yml` | Hardware evidence report form | GitHub, maintained | H, D | GitHub path convention | KEEP. |
| `.github/pull_request_template.md` | Pull-request validation checklist | GitHub, maintained | T, D | GitHub path convention | KEEP. |
| `.github/workflows/ci.yml` | Current format, build, test, diagnostics, distro, and cross-compile CI | CI, maintained | B, T | GitHub Actions path convention; invokes formatting, validation, Autotools, and compiler scripts | KEEP; workflow restructuring is outside a file-layout move. |
| `.gitignore` | Generated and local artifact exclusions | Development, maintained | B, D | `docs/generated-artifacts.md`, repository hygiene | KEEP at root. |
| `ARCHITECTURE.md` | Runtime and maintenance boundaries | Architecture, maintained | D | `README.md`, `Makefile.am` release archive | MOVE to `docs/architecture/architecture.md`; update root links and `EXTRA_DIST`. Low compatibility risk. |
| `CHANGELOG.md` | Maintained and inherited release history | Release, maintained | Rel, D | release scripts, version validator, `Makefile.am` | KEEP at root; conventional release metadata. |
| `CONTRIBUTING.md` | Contributor workflow and required validation | Development, maintained | T, D | `Makefile.am`; links formatting, validation, and evidence | KEEP at root; conventional contributor entry point. |
| `COPYING` | Complete GPLv3 text | Licensing, required | P, Rel, D | `README.md`, `LICENSE.md`, source lineage, `Makefile.am` install/archive | KEEP at root. Never merge or delete. |
| `DESIGN.md` | Long-term engineering principles | Architecture, maintained | D | `README.md`, `PROJECT.md`, `ARCHITECTURE.md`, `Makefile.am` | MOVE to `docs/architecture/design.md`; preserve links and archive inclusion. |
| `LICENSE.md` | Project license summary and file-specific caveats | Licensing, required | P, Rel, D | `README.md`, `NOTICE`, source lineage, release checklist, `Makefile.am` | KEEP at root. |
| `MAINTAINING.md` | Maintainer and release procedures | Development, maintained | T, Rel, D | `README.md`, release checklist, `Makefile.am` | MOVE to `docs/development/maintaining.md`; retain a prominent README link. |
| `Makefile.am` | Non-recursive Automake build, install, and archive manifest | Build, maintained | B, P, Rel | `configure.ac`, generated-artifact policy, all listed sources/support files | KEEP at root; update mechanically in layout PRs. |
| `NOTICE` | Copyright, attribution, and lineage notice | Licensing, required | P, Rel, D | `README.md`, `LICENSE.md`, source lineage, source headers, `Makefile.am` | KEEP at root. Never merge or delete. |
| `PROJECT.md` | Current scope and project status | Architecture, maintained | D | `DESIGN.md`, `Makefile.am` | MOVE to `docs/architecture/project-status.md`; low compatibility risk. |
| `README.md` | Primary user and contributor entry point | Documentation, maintained | P, Rel, D | installed documentation and `Makefile.am` archive | KEEP at root. |
| `RELEASE_CHECKLIST.md` | Release gate checklist | Release, maintained | Rel, T, H, D | `README.md`, `CONTRIBUTING.md`, `MAINTAINING.md`, `Makefile.am` | MOVE to `docs/release/checklist.md`; update all links and archive manifest. |
| `ROADMAP.md` | Completed and planned milestones | Release planning, maintained | D, Rel | release checklist, `Makefile.am` | MOVE to `docs/release/roadmap.md`. |
| `SECURITY.md` | Vulnerability reporting policy | Security, maintained | D, Rel | `Makefile.am`; GitHub convention | KEEP at root. |
| `VERSION` | Single maintained-project version source | Release, maintained | B, R, Rel | `configure.ac`, `version.h`, diagnostics, release/version scripts, tests, docs | KEEP at root; machine-readable contract. |
| `autogen.sh` | Git-checkout Autotools bootstrap | Build, maintained | B, T, P | README, contributing, maintaining, build/install validators | KEEP at root because the documented Git build contract is `./autogen.sh`. |
| `configure.ac` | Authoritative Autoconf metadata | Build, maintained | B, P, Rel | version validation, compatibility docs, generated `configure` | KEEP at root; enable `subdir-objects` during source move. |

## Runtime Source

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `mochad.c` | Daemon entry point, event loop, clients, USB lifecycle, listeners | Core, maintained/inherited | B, R, T, H | `Makefile.am`, setup/build validators, docs; includes all major subsystems | MOVE to `src/core/mochad.c`. Highest mechanical-risk file; behavior and symbols must remain unchanged. |
| `global.c` | Shared logging and global helpers | Core, maintained/inherited | B, R, T | `Makefile.am`, libusb-free compile, source lineage | MOVE to `src/core/global.c`. |
| `global.h` | Shared globals, logging, and daemon interfaces | Core, maintained/inherited | B, R, T | included by core, X10, USB, and tests; `Makefile.am` | MOVE to `src/core/global.h`; include paths must preserve flat include names. |
| `config.c` | Defaults, file/env/CLI merge, and validation | Configuration, maintained | B, R, T | `Makefile.am`, unit and TCP diagnostics validators | MOVE to `src/config/config.c`. |
| `config.h` | Validated configuration structure and constants | Configuration, maintained | B, R, T | config, diagnostics, daemon, tests, `Makefile.am` | MOVE to `src/config/config.h`. |
| `version.h` | Generated C version constants from `VERSION` | Configuration, generated-but-reviewed | B, R, Rel | diagnostics, daemon, release scripts, version validator, compatibility docs | MOVE to `src/config/version.h` and update the generator. Keep tracked because current version contract validates it. |
| `diagnostics.c` | Deterministic JSON diagnostic generation | Network diagnostics, maintained | B, R, T | `Makefile.am`, unit/TCP/version validators | MOVE to `src/net/diagnostics.c`. |
| `diagnostics.h` | Diagnostic structures and generator API | Network diagnostics, maintained | B, R, T | diagnostics, daemon, tests, `Makefile.am` | MOVE to `src/net/diagnostics.h`. |
| `socket_io.c` | Partial-write-safe socket send helpers | Network, maintained | B, R, T | `Makefile.am`, unit/TCP validators | MOVE to `src/net/socket_io.c`. |
| `socket_io.h` | Socket output API | Network, maintained | B, R, T | daemon, encoder, tests, `Makefile.am` | MOVE to `src/net/socket_io.h`. |
| `usb_endpoint_selection.c` | Claimed-interface interrupt endpoint selection | USB, maintained | B, R, T, H | daemon, unit tests, `Makefile.am` | MOVE to `src/usb/usb_endpoint_selection.c`. |
| `usb_endpoint_selection.h` | Endpoint-selection API and libusb dependency | USB, maintained | B, R, T, H | daemon, implementation, tests, `Makefile.am` | MOVE to `src/usb/usb_endpoint_selection.h`. |
| `x10_write.c` | Bounded X10 transmit queue and ACK progression | USB/X10 boundary, maintained/inherited | B, R, T, H | daemon, decoder, encoder, tests, compile scripts, source lineage | MOVE to `src/usb/x10_write.c`; retain filename and API to preserve review history. |
| `x10_write.h` | X10 transmit queue API | USB/X10 boundary, maintained/inherited | B, R, T, H | daemon, decoder, encoder, tests, `Makefile.am` | MOVE to `src/usb/x10_write.h`. |
| `decode.c` | CM15A/CM19A USB, RF, RFSEC, and power-line decoding | X10 protocol, maintained/inherited | B, R, T, H | daemon, encoder, tests, compile scripts, source lineage | MOVE to `src/x10/decode.c`; CM15A/CM19A and IDs `0bc7:0001/0002` remain protected behavior. |
| `decode.h` | Decode tables and API | X10 protocol, maintained/inherited | B, R, T | decoder, daemon, encoder, state, tests, source lineage | MOVE to `src/x10/decode.h`. |
| `encode.c` | Legacy command parser and RF/power-line encoding | X10 protocol, maintained/inherited | B, R, T, H | daemon, tests, compile scripts, source lineage | MOVE to `src/x10/encode.c`; ports and command behavior remain unchanged. |
| `encode.h` | Encoder/parser API | X10 protocol, maintained/inherited | B, R, T | decoder, daemon, tests, source lineage | MOVE to `src/x10/encode.h`. |
| `mochad_event.c` | Normalized event representation and legacy formatters | X10 protocol, maintained | B, R, T | decoder, unit tests, compile scripts, `Makefile.am` | MOVE to `src/x10/mochad_event.c`. |
| `mochad_event.h` | Normalized event model API | X10 protocol, maintained | B, R, T | decoder, implementation, tests, `Makefile.am` | MOVE to `src/x10/mochad_event.h`. |
| `x10state.c` | Legacy status/state representation | X10 protocol, maintained/inherited | B, R, T | daemon, decoder, encoder, tests, compile scripts, source lineage | MOVE to `src/x10/x10state.c`. |
| `x10state.h` | Legacy state API and data contracts | X10 protocol, maintained/inherited | B, R, T | daemon, decoder, encoder, tests, `Makefile.am` | MOVE to `src/x10/x10state.h`. |

## Tests And Test Support

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `tests/test_config.c` | Configuration precedence/default validation | Unit test, maintained | T | unit-test runner, `Makefile.am` archive | MOVE to `tests/unit/test_config.c`. |
| `tests/test_diagnostics.c` | Diagnostic JSON generation and bounds | Unit test, maintained | T | unit-test runner, `Makefile.am` | MOVE to `tests/unit/test_diagnostics.c`. |
| `tests/test_encode_rf.c` | RF encoding and `rf A2 on` regression | Unit test, maintained | T, H | unit-test runner, `Makefile.am` | MOVE to `tests/unit/test_encode_rf.c`. |
| `tests/test_mochad_event.c` | Golden native/XMLSocket/OpenRemote formatting | Unit/golden test, maintained | T | unit-test runner, `Makefile.am` | MOVE to `tests/golden/test_mochad_event.c`; preserved exact legacy framing coverage. |
| `tests/test_socket_io.c` | Partial write and EINTR behavior | Unit test, maintained | T | unit-test runner, `Makefile.am` | MOVE to `tests/unit/test_socket_io.c`. |
| `tests/test_tcp_diagnostics.c` | Loopback TCP diagnostic JSON transport | Component/integration test, maintained | T | TCP diagnostic validator, `Makefile.am` | MOVE to `tests/integration/test_tcp_diagnostics.c`. |
| `tests/test_usb_endpoint_selection.c` | Synthetic interface/altsetting/endpoint cases | Unit test, maintained | T, H | unit-test runner, `Makefile.am` | MOVE to `tests/unit/test_usb_endpoint_selection.c`. |
| `tests/test_x10_write.c` | Queue submission, failure, and ACK behavior | Unit test, maintained | T, H | unit-test runner, `Makefile.am` | MOVE to `tests/unit/test_x10_write.c`. |
| `tools/stubs/libusb-1.0/libusb.h` | Development-only libusb declaration stub | Test support, maintained | B, T | syntax validator, static-analysis docs, `Makefile.am` | MOVE to `tests/support/libusb-1.0/libusb.h`; never install or use at runtime. |

## Packaging, Installation, And Legacy Contributions

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `config/mochad.conf.example` | Inactive native configuration template | Linux packaging, maintained | P, T | `Makefile.am`, native setup tests | MOVE to `packaging/linux/config/mochad.conf.example`. |
| `systemd/mochad.service.in` | Rendered native systemd unit template | Linux packaging, maintained | P, T | setup tool, native setup tests, support/lineage docs, `Makefile.am` | MOVE to `packaging/linux/systemd/mochad.service.in`. |
| `systemd/mochad.service` | Fixed-path historical systemd unit | Linux packaging, legacy | Rel, D | `EXTRA_DIST`, native setup test reference check, support/lineage docs | ARCHIVE AS RESEARCH under `docs/research/legacy-packaging/systemd/`; superseded by `.in`, but retain provenance. |
| `udev/91-usb-x10-controllers.rules.in` | Rendered CM15A/CM19A permission rule | Linux packaging, maintained | P, T, H | setup tool/tests, support/lineage docs, `Makefile.am` | MOVE to `packaging/linux/udev/`. |
| `udev/91-usb-x10-controllers.rules` | Fixed-group historical permission rule | Linux packaging, legacy | Rel, D | `EXTRA_DIST`, setup test reference check, source-lineage docs | ARCHIVE AS RESEARCH; `.in` is authoritative and covers both USB IDs. |
| `udev/91-usb-x10-controllers.rules-systemd` | Historical rule that starts systemd service | Linux packaging, legacy/superseded | Rel, D | `EXTRA_DIST`, source lineage | ARCHIVE AS RESEARCH; current policy explicitly separates permissions from service activation. |
| `udev/README.md` | Current template guidance plus obsolete rule note | Linux packaging docs, mixed | P, D | `EXTRA_DIST`, support docs | MERGE maintained guidance into `docs/installation/native-linux.md`; archive the historical rule note with legacy packaging. |
| `hotplug2/20-usb-x10` | OpenWrt hotplug start hook for both USB IDs | OpenWrt packaging, legacy compatibility | P, H | installed as legacy data and in `EXTRA_DIST`; support/lineage docs | MOVE to `packaging/openwrt/hotplug2/20-usb-x10`; do not delete without OpenWrt evidence. |
| `hotplug2/mochad` | OpenWrt `/etc/rc.common` init script | OpenWrt packaging, legacy compatibility | P | installed as legacy data, ShellCheck, support/lineage docs | MOVE to `packaging/openwrt/init.d/mochad`; preserve semantics. |
| `scripts/install-native.sh` | Source-tree quick native install wrapper | Setup, maintained | P, T | README, changelog, `Makefile.am` | MOVE to `scripts/setup/install-native.sh`; provide or document the new invocation. |
| `scripts/mochad-redux-setup.in` | Installed host-administration tool template | Setup, maintained | P, T | `configure.ac`, `Makefile.am`, setup/staged-install tests, docs | MOVE to `scripts/setup/mochad-redux-setup.in`; generated destination remains controlled by Automake. |
| `apps/bash.sh` | Site-specific DS10A-to-module/TTS example | Contribution, legacy | D | `EXTRA_DIST`, support docs | MOVE to `contrib/apps/bash.sh`; preserve as non-authoritative GPL-era example. |
| `apps/mochad.scr` | MisterHouse raw FIFO helper | Contribution, legacy | D | `EXTRA_DIST`, support docs | MOVE to `contrib/apps/mochad.scr`; NEEDS REVIEW before execution due `killall`, fixed paths, and obsolete udev advice. |
| `apps/mochamon.pl` | Multi-daemon event monitor | Contribution, legacy | D | `EXTRA_DIST`, support docs | MOVE to `contrib/apps/mochamon.pl`; hardcoded hosts make it an example only. |
| `apps/rfsectopl3.pl` | RF security to power-line/TTS example | Contribution, legacy | D | `EXTRA_DIST`, support docs | MOVE to `contrib/apps/rfsectopl3.pl`; retain GPL notice and mark site-specific. |
| `apps/simplemon.pl` | Minimal legacy TCP monitor | Contribution, legacy | D | `EXTRA_DIST`, support docs | MOVE to `contrib/apps/simplemon.pl`. |
| `apps/x10-tk.py` | Python 2/Tk control demo | Contribution, obsolete runtime but useful history | D | `EXTRA_DIST`, changelog, support docs | ARCHIVE AS RESEARCH under `docs/research/legacy-apps/`; Python 2 and hardcoded host prevent maintained-example status. |
| `cgi/cgi-lib.pl` | Third-party CGI parser 2.18 | Contribution, legacy/provenance-sensitive | D, Rel | CGI entrypoint, `EXTRA_DIST`, `NOTICE`, source lineage | MOVE to `contrib/cgi/cgi-lib.pl`; preserve embedded custom permission notice exactly. |
| `cgi/getsensors.pl` | Legacy CGI status renderer | Contribution, legacy | D | `cgi/x10.pl`, `EXTRA_DIST`, support docs | MOVE to `contrib/cgi/getsensors.pl`; do not advertise as secure. |
| `cgi/netcat.pl` | Legacy Perl TCP helper | Contribution, legacy | D | `cgi/x10.pl`, `EXTRA_DIST`, support docs | MOVE to `contrib/cgi/netcat.pl`. |
| `cgi/x10.pl` | Site-specific, unauthenticated CGI control page | Contribution, obsolete for deployment | D | requires three CGI helpers; `EXTRA_DIST`, support docs | ARCHIVE AS RESEARCH with the CGI set unless a maintainer commits to security ownership. Never install or recommend. |

No `packaging/macos/` inputs exist at this base. Do not create an empty
directory. macOS remains a development/compiler compatibility target rather
than a native packaging target.

## Scripts

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `scripts/format-c.sh` | Apply repository C format policy | Formatting, maintained | T, D | contributing docs and `.clang-format*` | MOVE to `scripts/format/c.sh`; retain a clear documented command. |
| `scripts/release/prepare-next-dev.sh` | Start next development version | Release, maintained | Rel | compatibility docs, version updater, `Makefile.am` | KEEP under `scripts/release/`. |
| `scripts/release/prepare-release.sh` | Prepare release version/evidence | Release, maintained | Rel | compatibility docs, version updater, `Makefile.am` | KEEP. |
| `scripts/release/update-version-files.sh` | Generate `version.h` from `VERSION` | Release, maintained | B, Rel | both preparation scripts, validator, docs, `Makefile.am` | KEEP but update generated header path. |
| `scripts/validate/clang-format.sh` | Verify maintained C formatting | Validation, maintained | T | CI, contributing/static-analysis docs | KEEP under `scripts/validate/`; update source paths. |
| `scripts/validate/clean-build-test.sh` | Remove ignored outputs then run clean builds | Validation, maintained | B, T | validation docs, README, `Makefile.am` | KEEP; destructive scope remains ignored files only. |
| `scripts/validate/cm19a-hardware-validation.sh` | Interactive CM19A release procedure | Hardware, maintained | H, Rel | validation docs/evidence/checklist, `Makefile.am` | MOVE to `scripts/hardware/cm19a-validation.sh`; update evidence commands. |
| `scripts/validate/docker-build.sh` | Compatibility build through mochad-docker | Validation, maintained | B, T, Rel | validation docs/evidence/checklist, `Makefile.am` | KEEP; external repo availability is an explicit requirement. |
| `scripts/validate/docker-smoke-test.sh` | Docker command-line smoke validation | Validation, maintained | T, Rel | validation docs/evidence/checklist, `Makefile.am` | KEEP. |
| `scripts/validate/full-libusb-build.sh` | Autotools/libusb build | Build validation, maintained | B, T | other validators, validation docs/checklist, `Makefile.am` | KEEP. |
| `scripts/validate/libusb-stub-syntax-check.sh` | Full-daemon syntax check with local stub | Build validation, maintained | B, T | README, validation docs, `Makefile.am` | KEEP; update stub and source paths. |
| `scripts/validate/native-setup-tool.sh` | Isolated host-setup contract tests | Install validation, maintained | P, T | contributing/maintaining/evidence/checklist, `Makefile.am` | KEEP; update packaging/template paths. |
| `scripts/validate/native-smoke-test.sh` | CLI/version/config smoke tests | Runtime validation, maintained | T, Rel | clean build, validation docs/checklist, `Makefile.am` | KEEP. |
| `scripts/validate/release-evidence.sh` | Validate allowed evidence statuses | Release validation, maintained | Rel | release checklist, `Makefile.am` | KEEP. |
| `scripts/validate/repository-hygiene.sh` | Reject dirty trees and tracked artifacts | Repository validation, maintained | T, Rel | release checklist, evidence, `Makefile.am` | KEEP; extend expected layout rules after moves. |
| `scripts/validate/shellcheck.sh` | ShellCheck tracked maintained scripts | Validation, maintained | T | CI, contributing/static-analysis docs | KEEP; update included/excluded path policy. |
| `scripts/validate/source-archive-validation.sh` | Export, compile, and stage-install source tree | Release validation, maintained | B, P, Rel | release checklist/evidence, `Makefile.am` | KEEP; strongest replacement coverage for archive path moves. |
| `scripts/validate/staged-install-contract.sh` | Prove `DESTDIR` install is pure | Install validation, maintained | P, T, Rel | source archive, contributing/maintaining/evidence/checklist, `Makefile.am` | KEEP. |
| `scripts/validate/strict-libusb-free-compile.sh` | Strict source-only compile wrapper | Build validation, maintained | B, T | build/source-archive validators, docs/checklist, `Makefile.am` | KEEP; update compiler tool path. |
| `scripts/validate/tcp-diagnostics-smoke-test.sh` | Compile/run/JSON-validate TCP diagnostics | Integration validation, maintained | T | CI, README, validation docs, `Makefile.am` | KEEP; update source/test paths. |
| `scripts/validate/unit-tests.sh` | Compile and execute focused C tests | Test runner, maintained | T | CI, README, validation docs, `Makefile.am` | KEEP; update unit/golden paths. |
| `scripts/validate/version-consistency.sh` | Cross-check all version identities | Release validation, maintained | B, R, Rel | preparation scripts, README, `Makefile.am` | KEEP; update `version.h`, docs, and evidence paths. |
| `tools/compile_without_libusb.sh` | Main strict/static-analysis/sanitizer compiler harness | Build tool, maintained | B, T, Rel | strict wrapper, CI/docs/evidence/checklist, `Makefile.am` | MOVE to `scripts/build/compile-without-libusb.sh`; retain options and output behavior. |

The validation scripts overlap by orchestration, not by exact implementation:
small wrappers compose the compiler harness, full build, archive, install, and
hardware procedures. They should not be merged during a layout-only change.

## Documentation And Validation Records

| Current path | Purpose | Owner/status | Participation | Live references | Proposed action and evidence |
| --- | --- | --- | --- | --- | --- |
| `docs/beta-status.md` | Beta scope and evidence language | Release docs, maintained | D, Rel | README | MOVE to `docs/release/beta-status.md`. |
| `docs/compatibility.md` | Redux/upstream version and compatibility contract | Architecture docs, maintained | B, Rel, D | README, version files/scripts, `Makefile.am` | MOVE to `docs/architecture/compatibility.md`. |
| `docs/generated-artifacts.md` | Generated-file policy | Development docs, maintained | B, Rel, D | maintaining/release docs, `.gitignore`, `Makefile.am` | MOVE to `docs/development/generated-artifacts.md`. |
| `docs/hardware-validation.md` | Repeatable CM15A/CM19A test procedure | Installation/testing docs, maintained | H, Rel, D | README, validation docs, `Makefile.am` | MOVE to `docs/development/hardware-validation.md`. |
| `docs/json-api.md` | Design-only future JSON-RPC specification | Protocol design, maintained but unimplemented | D | README, project, architecture, roadmap, `Makefile.am` | MOVE to `docs/protocol/json-api.md`; continue labeling port 1102 unimplemented. |
| `docs/milestone-1.md` | Completed historical maintenance milestone | Historical docs | D | `Makefile.am`; references current build commands | ARCHIVE AS RESEARCH under `docs/research/milestones/`. |
| `docs/mochad-redux-setup.8` | Setup administration tool man page | Linux packaging docs, maintained | P, D | `Makefile.am` install/archive | MOVE to `packaging/linux/man/mochad-redux-setup.8`. |
| `docs/mochad.1` | Daemon man page | Linux packaging docs, maintained | P, D | `Makefile.am` install/archive | MOVE to `packaging/linux/man/mochad.1`. |
| `docs/native-install-rollback.md` | Explicit host rollback procedure | Installation docs, maintained | P, T, D | README, architecture, `Makefile.am` | MOVE to `docs/installation/native-install-rollback.md`. |
| `docs/source-lineage.md` | Upstream, support-file, CGI, and license provenance | Research/licensing, required | P, Rel, D | README, LICENSE, NOTICE, `Makefile.am` | MOVE to `docs/research/source-lineage.md`; update source archive expectations. |
| `docs/static-analysis-audit.md` | Closed static-analysis findings and policy evidence | Development evidence, historical/current | T, D | contributing docs | MOVE to `docs/research/audits/static-analysis.md`; retain as evidence. |
| `docs/static-analysis-cleanup-plan.md` | Completed static-analysis implementation plan | Historical planning | D | no live reference beyond Git history | ARCHIVE AS RESEARCH under `docs/research/plans/`; do not treat as current instructions. |
| `docs/support-files.md` | Status of apps, CGI, OpenWrt, udev, and systemd | Development docs, maintained | P, D | README, `Makefile.am` | MOVE to `docs/development/legacy-support-files.md`; revise after support-file moves. |
| `docs/supported-platforms.md` | Linux-first and compatibility support policy | Installation docs, maintained | B, P, Rel, D | README, maintaining, `Makefile.am` | MOVE to `docs/installation/supported-platforms.md`. |
| `validation/README.md` | Validation framework index and evidence rules | Validation docs, maintained | T, Rel, H | README, `Makefile.am` | KEEP. |
| `validation/compiler.md` | Compiler/static-analysis evidence checklist | Validation docs, maintained | B, T, Rel | `Makefile.am` and validation scripts | KEEP. |
| `validation/hardware.md` | Hardware evidence requirements | Validation docs, maintained | H, Rel | contributing docs, `Makefile.am` | KEEP; link moved hardware script. |
| `validation/regression.md` | Protected runtime/legacy regression list | Validation docs, maintained | T, Rel | `Makefile.am` | KEEP; explicitly protects ports 1099, 1100, and 1101. |
| `validation/release-evidence-template.md` | Allowed-status release record template | Release evidence, maintained | Rel, H | release scripts/checklist/maintaining/contributing, `Makefile.am` | KEEP. |
| `validation/releases/README.md` | Release evidence directory policy | Release evidence, maintained | Rel | `Makefile.am` | KEEP. |
| `validation/releases/v0.4.0.md` | v0.4.0 evidence record | Release evidence, immutable historical record | Rel, H | version validator, release scripts, `Makefile.am` | KEEP at the same path; commands may receive a dedicated path-update note rather than rewriting historical results. |

## Compatibility Findings

- Main native TCP port 1099, XMLSocket port 1100, and OpenRemote port 1101 are
  runtime behavior in `mochad.c`, `config.c`, `encode.c`, normalized event
  formatters, tests, and regression documentation. None is obsolete.
- CM15A and CM19A handling and USB IDs `0bc7:0001` and `0bc7:0002` remain live
  in runtime, endpoint, packaging, setup, hardware, and documentation paths.
- `hotplug2/` is old but remains the only tracked OpenWrt integration. Its age
  is not evidence for deletion.
- `version.h` is generated, but deliberately tracked and verified against
  `VERSION`; changing that contract requires a separate release decision.
- Release archives currently derive their manifest from `Makefile.am`
  `EXTRA_DIST`, while Git source validation exports tracked files. Every move
  must update both assumptions.
- `COPYING`, `LICENSE.md`, `NOTICE`, and source-lineage documentation are
  complementary, not duplicate licensing files.

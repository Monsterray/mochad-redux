# Repository Layout Proposal

Audit base: `3c387bda4c308f330c7fb825af7de9baf63af69e`

This proposal turns the file inventory into a reviewable target. It does not
authorize deletions and does not require empty directories or new modules.

## Goals

- Make runtime ownership visible without changing C symbols or behavior.
- Separate maintained packaging from inherited historical examples.
- Keep native, source-archive, Docker-source, and Git-checkout build contracts.
- Keep repository-root release and community entry points conventional.
- Preserve Git rename detection with mechanical commits.
- Make provenance-sensitive material clearly non-authoritative.

## Proposed Populated Tree

```text
.
├── .github/
├── src/
│   ├── core/
│   │   ├── mochad.c
│   │   ├── global.c
│   │   └── global.h
│   ├── config/
│   │   ├── config.c
│   │   ├── config.h
│   │   └── version.h
│   ├── net/
│   │   ├── diagnostics.c
│   │   ├── diagnostics.h
│   │   ├── socket_io.c
│   │   └── socket_io.h
│   ├── usb/
│   │   ├── usb_endpoint_selection.c
│   │   ├── usb_endpoint_selection.h
│   │   ├── x10_write.c
│   │   └── x10_write.h
│   └── x10/
│       ├── decode.c
│       ├── decode.h
│       ├── encode.c
│       ├── encode.h
│       ├── mochad_event.c
│       ├── mochad_event.h
│       ├── x10state.c
│       └── x10state.h
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── golden/
│   └── support/libusb-1.0/
├── packaging/
│   ├── linux/
│   │   ├── config/
│   │   ├── man/
│   │   ├── systemd/
│   │   └── udev/
│   └── openwrt/
│       ├── hotplug2/
│       └── init.d/
├── scripts/
│   ├── build/
│   ├── format/
│   ├── hardware/
│   ├── release/
│   ├── setup/
│   └── validate/
├── docs/
│   ├── architecture/
│   ├── development/
│   ├── installation/
│   ├── protocol/
│   ├── release/
│   └── research/
├── contrib/
│   ├── apps/
│   └── cgi/
└── validation/
    └── releases/
```

`src/compat/`, `packaging/macos/`, `contrib/examples/`,
`tests/fixtures/`, and `validation/hardware-runs/` are intentionally absent
until a reviewed file actually belongs there.

## Root Files

Keep these root-level contracts:

- `README.md`
- `CHANGELOG.md`
- `CONTRIBUTING.md`
- `SECURITY.md`
- `VERSION`
- `COPYING`
- `LICENSE.md`
- `NOTICE`
- `Makefile.am`
- `configure.ac`
- `autogen.sh`
- `.clang-format`
- `.clang-format-ignore`
- `.gitignore`

`AUTHORS` does not currently exist. Do not manufacture one until source
lineage is complete enough to make it accurate.

## Source Ownership

### Core

`src/core/` owns process startup, shutdown, the event loop, client lifecycle,
shared globals, and logging. Moving `mochad.c` does not authorize splitting it.

### Configuration

`src/config/` owns the validated configuration structure and generated runtime
version constants. `VERSION` remains the editable root source; only the
generated header moves.

### Network

`src/net/` owns socket output and diagnostic JSON. Legacy native,
XMLSocket, and OpenRemote formatting remains in the current event/protocol
implementation; this move does not redesign listener boundaries.

### USB

`src/usb/` owns endpoint selection, transfer-facing output, and the X10 transmit
queue. `x10_write` stays intact because its USB/controller-ACK coupling is a
protected regression area.

### X10

`src/x10/` owns decode, encode, normalized events, legacy output formatting,
and daemon state representation. Standard RF identities and security RF
identifiers remain unchanged.

No source file belongs in `src/compat/` yet. Platform headers and conditional
code are small and embedded in their owning modules. Moving them into a new
module would be source refactoring rather than repository organization.

## Build-System Changes

The source-layout branch should make only these mechanical build changes:

1. Add `subdir-objects` to `AM_INIT_AUTOMAKE`.
2. Update `mochad_SOURCES` to the new paths.
3. Add explicit include directories through `AM_CPPFLAGS` so existing flat
   quoted includes continue to compile without edits where possible.
4. Restore `AC_CONFIG_SRCDIR([src/core/mochad.c])`.
5. Update source lists in strict compile, syntax, unit, TCP, formatting,
   version, source-archive, and staged-install validators.
6. Update the version generator to write `src/config/version.h`.
7. Do not commit generated `configure`, `Makefile.in`, dependency, or object
   files.

The non-recursive root `Makefile.am` remains authoritative. No recursive
subdirectory Makefiles are proposed.

## Support And Packaging

- Maintained native templates move under `packaging/linux/`.
- OpenWrt-era hotplug/init files move under `packaging/openwrt/` and remain
  legacy compatibility inputs.
- The native setup implementation remains under `scripts/setup/`; it renders
  installed templates and remains the only supported live-host integration
  path.
- Historical fixed systemd/udev files move into a clearly labeled research
  archive. They must not remain in `packaging/linux/`, where users could mistake
  them for current templates.
- Legacy applications and CGI code move under `contrib/` only if retained as
  runnable examples. Unsafe or unsupported examples should instead be archived
  under `docs/research/legacy-*`.

The support-layout PR must preserve:

- pure `make install`;
- `DESTDIR`;
- `mochad-redux-setup`;
- the installed `mochad.service` name;
- rendered custom user/group/prefix behavior;
- release archive membership;
- Docker builds that consume an exact source SHA.

## Test Ownership

- `tests/unit/`: isolated function/module behavior.
- `tests/golden/`: byte/text compatibility outputs, including all three legacy
  listener framings.
- `tests/integration/`: real loopback/socket/process boundaries.
- `tests/support/`: test-only declarations and helpers.

The current tests require no new fixtures directory. The golden event test
contains literals in C and remains deterministic.

## Documentation Ownership

- `docs/architecture/`: runtime boundaries, principles, compatibility, status.
- `docs/development/`: maintainer procedures, repository policy, hardware
  validation, support-file ownership.
- `docs/installation/`: platform support, native installation, rollback.
- `docs/protocol/`: implemented protocol documentation and explicitly
  design-only JSON-RPC specification.
- `docs/release/`: beta status, roadmap, release checklist.
- `docs/research/`: completed plans, historical milestones, source lineage, and
  non-authoritative inherited support artifacts.

Historical release evidence under `validation/releases/` should not be silently
rewritten to pretend old commands used new paths. Add a relocation note when
necessary and keep recorded results intact.

## Branch And Commit Sequence

### `refactor/repository-layout-source`

Base it on the approved audit base. Use one commit for source/header `git mv`
operations and a second for mechanical build/test path updates. Do not include
support moves, deletions, symbol changes, or formatting churn.

Required validation:

- ClangFormat
- strict compile
- unit tests
- TCP diagnostics
- legacy protocol golden tests
- version consistency
- source archive validation
- repository hygiene

### `refactor/repository-layout-support`

Base it on the merged source-layout result. Move tests, support, packaging,
scripts, validation references, and documentation categories. Separate
mechanical moves from reference/documentation updates.

Required validation adds:

- ShellCheck
- full libusb build
- staged `DESTDIR` install
- native setup validation
- Docker source-build compatibility where available

### `cleanup/remove-obsolete-files`

Base it on the merged support-layout result. Delete only approved files from
the obsolete-file report. Each deletion gets an adjacent documentation entry
with replacement coverage and provenance handling.

No deletion is approved merely by this proposal.

### `docs/repository-architecture`

Finalize public paths, diagrams, contributor commands, installation paths,
source-lineage references, and release guidance after the mechanical branches
settle. Do not mix source moves into this branch.

## Compatibility Gates

Every phase must explicitly preserve:

- `./autogen.sh` for Git checkouts;
- release archives that include generated `configure`;
- `./configure && make`;
- pure `make install` and staged `DESTDIR`;
- explicit `mochad-redux-setup`;
- `mochad --version` and diagnostic version identity;
- ports 1099, 1100, and 1101 and their framing;
- CM15A and CM19A enumeration and packet handling;
- USB IDs `0bc7:0001` and `0bc7:0002`;
- OpenWrt files unless separately retired with platform evidence;
- licenses, notices, original headers, and source lineage.

Pure path moves are `NOT APPLICABLE` for physical hardware behavior unless a
USB, startup, shutdown, transmission, or process-control path changes. If such
a change appears accidentally, stop the layout PR and classify hardware
validation as `HARDWARE REQUIRED`.

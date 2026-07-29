# Documentation

Use this page to distinguish current guidance from historical research.

## Architecture

- [Architecture](architecture/architecture.md): runtime and repository
  boundaries.
- [Design principles](architecture/design.md): compatibility and maintenance
  policy.
- [Compatibility](architecture/compatibility.md): maintained and upstream
  version identities.
- [Project status](architecture/project-status.md): current scope and
  non-goals.

## Installation

- [Supported platforms](installation/supported-platforms.md)
- [Native rollback](installation/native-install-rollback.md)
- [Root README](../README.md): build and installation entry point.

## Development

- [Maintainer guide](development/maintaining.md)
- [Generated-artifact policy](development/generated-artifacts.md)
- [Hardware validation](development/hardware-validation.md)
- [Hardware lab setup](development/hardware-lab-setup.md)
- [Test strategy](development/test-strategy.md)
- [Legacy support-file ownership](development/legacy-support-files.md)
- [Repository layout record](development/repository-layout-proposal.md)
- [Tracked-file inventory](development/repository-file-inventory.md)
- [Duplicate and obsolete-file audit](development/duplicate-and-obsolete-files.md)
- [Branch closeout inventory](development/branch-closeout-inventory.md)
- [Branch closeout plan](development/branch-closeout-plan.md)
- [Branch closeout results](development/branch-closeout-results.md)

The inventory and obsolete-file audit are records of the pre-move audit base.
They retain historical paths intentionally.

## Operations

- [Sanitized support bundles](operations/support-bundles.md)

## Protocol

- [JSON API design](protocol/json-api.md): design-only future work, not an
  implemented listener.
- [Transport evidence](operations/transport-evidence.md): bounded Redux-owned
  command, USB, acknowledgement, and decode facts.

The implemented native TCP, XMLSocket, and OpenRemote behavior is documented
in the root README and architecture guide.

## Release

- [Beta status](release/beta-status.md)
- [Release checklist](release/checklist.md)
- [Roadmap](release/roadmap.md)
- [Validation evidence](../validation/README.md)

## Research

[Research](research/README.md) contains completed plans, historical milestones,
source-lineage evidence, and superseded support artifacts. Research files are
not current installation instructions and must not be executed merely because
they are present in the source archive.

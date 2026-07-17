# Compatibility

`mochad-redux` is a maintained compatibility-focused fork of the legacy
`mochad` daemon. The maintained project version is the plain semantic version
in the repository-root `VERSION` file. The historical upstream baseline is a
separate identity: `mochad 0.1.18`.

| mochad-redux version | Upstream base | Compatibility intent |
| --- | --- | --- |
| 0.4.0 | mochad 0.1.18 | Preserve the native TCP listener and legacy XMLSocket/OpenRemote behavior while applying documented maintenance fixes. |

The project version is not an upstream version. TCP diagnostics therefore
expose separate machine-readable `name`, `version`, and `upstream_base` fields.

## Version Contract

- `VERSION` is the one editable source of the maintained version.
- Version files use semantic versions without `v`: `0.5.0`, `0.5.0-dev`, or
  `0.5.0-rc1`.
- Git tags use the same version with a leading `v`, such as `v0.5.0`.
- `version.h` is generated from `VERSION` with
  `scripts/release/update-version-files.sh`.
- `configure.ac` reads `VERSION` directly, so Autotools package metadata and
  the C runtime stay aligned.

For a human-operated release, run `scripts/release/prepare-release.sh 0.5.0`.
For the next development cycle, run
`scripts/release/prepare-next-dev.sh 0.6.0`. Both scripts require a clean
working tree and only prepare reviewable files; neither creates a tag, pushes,
or publishes anything.

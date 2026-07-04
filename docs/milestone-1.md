# Milestone 1: Maintenance Foundation

Milestone 1 should make the fork dependable to build, inspect, and maintain
before protocol or behavioral changes are introduced.

## Goals

- Confirm the upstream relationship and keep history rebase-friendly.
- Keep `develop` as the ongoing integration branch.
- Add CI that verifies the autotools build on a clean Linux runner.
- Document the local build path and required dependencies.
- Improve diagnostics and configuration only where behavior remains compatible.

## Non-goals

- No protocol changes.
- No topic, command, or X10 behavior changes.
- No large rewrites.
- No vendoring of external dependencies.

## Acceptance checks

- `git fetch upstream --tags` succeeds.
- `develop` can be compared directly with `upstream/master`.
- `./autogen.sh`, `./configure`, and `make` pass in CI.
- Local build failures clearly identify missing host dependencies.
- Any diagnostics or configuration changes are documented before release.

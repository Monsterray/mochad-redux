# Design Principles

`mochad-redux` is an independently maintained continuation of `mochad`. Its
primary objective is long-term compatibility, reliability, and maintainability
for real X10 CM15A and CM19A installations.

The project may reuse, cherry-pick, or learn from upstream forks, but it does
not aim for commit-for-commit parity with any upstream repository.

## Compatibility First

Existing `mochad` TCP clients should continue to work unless there is a clear,
documented reason to break compatibility.

Compatibility includes:

- Existing command syntax.
- Existing TCP ports and line-oriented protocol behavior.
- Existing CM15A and CM19A controller expectations.
- Existing deployment patterns where practical.

Breaking changes require explicit review, migration notes, and a release note.

## Behavior Before Features

Prefer making existing behavior safer, clearer, more testable, and better
documented before adding new functionality.

Good changes:

- Fix a concrete crash, leak, or bounds issue.
- Make startup or shutdown failures easier to diagnose.
- Add tests or validation steps for behavior that already exists.
- Document hardware behavior that was previously implicit.

Riskier changes:

- Add new protocol commands before the existing paths are validated.
- Change runtime defaults without a compatibility reason.
- Introduce large abstractions around code that is not yet covered by tests.

For the `v0.4.x` line, runtime observability is the primary feature. Startup,
shutdown, listener, USB, and client lifecycle logs should become clear enough
that a user can diagnose most deployment failures from logs alone. New protocol
features should wait until that visibility is in place.

## Linux-First Runtime

Linux is the primary runtime platform. The reference behavior is Linux with
`libusb` and real CM15A or CM19A hardware.

Other environments can still be useful:

- macOS can support compile-only checks for non-USB code.
- CI can validate libusb-free translation units.
- Cross-compilation can catch portability issues.

Those checks are valuable, but they do not replace Linux hardware validation.

## Docker Is First-Class

Docker is a supported deployment model, not an afterthought.

Docker behavior should remain:

- Foreground by default.
- Friendly to container logs.
- Explicit about USB passthrough requirements.
- Compatible with host networking and normal TCP clients.

Docker should not hide USB errors. If the controller cannot be opened, logs
should make the likely cause visible.

## Explicit Privilege Boundaries

Build and package installation must be safe in release archives, package
staging roots, containers, and CI. `make install` therefore only copies files
and honors `DESTDIR`. Account creation, udev reloads, systemd changes, service
activation, and USB access belong only to an explicit native-administration
step. The managed `mochad-redux-setup` tool protects local edits and preserves
configuration by default; it is never part of `mochad-docker` runtime setup.

## Incremental Maintenance

Large rewrites are discouraged. Changes should be small, reviewable, and easy
to reason about.

Prefer:

- One safety issue per patch.
- One diagnostic improvement per patch.
- Documentation updates beside behavior changes.
- Compatibility-preserving edits over architectural churn.

Avoid broad USB, TCP, protocol, or state separation unless the behavior is
already covered by tests or a focused manual validation path.

## Testability

Protocol logic should be testable without USB hardware whenever practical.

The project should maintain two validation paths:

1. Full Linux build and hardware validation with `libusb`.
2. libusb-free compile and analyzer checks for protocol-adjacent code.

Manual hardware validation is also part of the test strategy. Repeatable CM19A
and CM15A test reports are more valuable than speculative refactors.

After runtime logging is stable, sanitizer builds, static analysis, and
compatibility documentation should be improved before starting larger
modernization work.

## Upstream Posture

`mochad-redux` should stay aware of upstream work without being defined by it.

Use upstream projects as sources of:

- Historical context.
- Bug fixes worth cherry-picking.
- Compatibility expectations.
- Prior art for diagnostics or packaging.

Do not treat upstream divergence as a defect by itself. The maintained line is
allowed to differ when doing so improves clarity, reliability, packaging, or
long-term maintenance.

## Versioning

Use semantic versioning for `mochad-redux` releases:

- `v0.4.x`: runtime hardening and observability releases.
- `v0.5.0`: next milestone.
- `v1.0.0`: mature stable release with documented hardware support.

The inherited upstream package version may remain visible as the upstream base,
but user-facing `mochad-redux` releases should use the fork's version tags.

## Review Checklist

Before accepting a change, ask:

- Does this preserve existing `mochad` behavior?
- Is the change smaller than it could reasonably be?
- Is the user-facing behavior documented?
- Can any part of this be tested without hardware?
- If hardware is required, is the validation path documented?
- Does this make Docker deployment clearer or at least avoid making it worse?
- Does this make runtime logs clearer when startup, USB, TCP, client, or
  shutdown behavior fails?
- Does this improve maintainability without chasing abstraction for its own sake?

If a change fails one of these checks, it may still be correct, but the reason
should be explicit in the pull request and release notes.

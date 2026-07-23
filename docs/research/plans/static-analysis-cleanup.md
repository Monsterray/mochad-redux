# Static Analysis Cleanup Plan

This document tracks the cross-repository static-analysis cleanup. It is a
maintenance plan, not a release claim.

## Scope

- `mochad-redux`: C formatting and shell scripts.
- `mochad-docker`: maintained shell scripts.
- `mochad-mqtt-bridge`: maintained shell scripts.

## Sequence

1. Record a read-only inventory and classify every finding.
2. Define a project-owned C formatting policy for `mochad-redux`.
3. Keep formatting-only changes separate from behavioral fixes.
4. Correct ShellCheck safety defects before style-only findings.
5. Add deterministic validators, CI gates, and developer documentation.
6. Run the affected test suites and publish an audit disposition report.

## Current Inventory

- Redux has 30 maintained C/header files and one development libusb stub.
- Redux has 26 tracked shell scripts; Docker has 8; Bridge has 6.
- Redux has two known real `autogen.sh` defects: unchecked `cd` (`SC2164`)
  and unsafe `$*` execution (`SC2048`).
- The current source has no `.clang-format`; unnamed clang-format defaults are
  not an accepted project policy.

## Guardrails

- Do not reformat generated, vendored, or legacy compatibility files without a
  documented decision.
- Do not add broad lint suppressions.
- Do not mix mechanical C formatting with C behavior changes.
- Normal CI must fail on newly introduced formatting or ShellCheck drift.

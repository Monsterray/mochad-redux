# mochad-redux Agent Instructions

The workspace `AGENTS.md` defines the minimum safety boundary. This file adds
repository-specific validation commands and does not loosen workspace policy.

## Source validation

Use this order:

```text
scripts/validate/clang-format.sh
scripts/validate/shellcheck.sh
scripts/validate/strict-libusb-free-compile.sh
MOCHAD_SANITIZERS=disabled scripts/validate/unit-tests.sh
scripts/validate/tcp-diagnostics-smoke-test.sh
scripts/validate/full-libusb-build.sh
scripts/validate/staged-install-contract.sh
scripts/validate/native-setup-tool.sh
scripts/validate/source-archive-validation.sh
scripts/validate/version-consistency.sh
scripts/validate/repository-hygiene.sh
```

Enable ASan/UBSan explicitly for release evidence:

```sh
MOCHAD_SANITIZERS=enabled scripts/validate/unit-tests.sh
```

## Hardware boundary

Hardware validation is manual and approval-gated:

- use the restricted `codex-x10` account;
- never grant it `sudo`, Docker, production service, or production MQTT access;
- use `/run/lock/x10-hardware.lock`;
- bind only localhost ports in `19000-19999`;
- use X10 housecode `D` only;
- run preflight before opening the controller;
- display every transmitting command and record explicit human approval;
- report unobserved physical outcomes as `HARDWARE REQUIRED`.

Start with:

```sh
scripts/hardware/lab-preflight.sh
scripts/hardware/run-cm19a-validation.sh
```

The second command does not transmit unless `--transmit` is supplied and the
exact approval phrase is entered interactively.

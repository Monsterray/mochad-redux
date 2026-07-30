# mochad-redux Agent Instructions

Workspace `AGENTS.md` defines safety, evidence, machine, and Git boundaries.
This file adds Redux-specific invariants.
In the shared workspace, conditional routing is under `../docs/agent/`.

## Invariants

- Redux owns USB/controller access, X10 wire encoding and decoding, normalized
  events, TCP services, diagnostics, and native installation.
- Keep MQTT, Home Assistant, Docker policy, and device-model semantics outside
  Redux unless a distinction is necessary for protocol decoding.
- Preserve existing native newline, XMLSocket NUL, and OpenRemote framing.
- Preserve default ports, CLI/config precedence, version identity, and
  behavior unless the task explicitly changes the documented contract.
- Keep `make install` copy-only and `DESTDIR`-safe. Host integration belongs to
  `mochad-redux-setup`.
- Transport completion or controller acknowledgement must not claim physical
  device activation.

## Validation Entry Points

Use `.validation/capabilities.json` to select the profile.

Fast source validation starts with:

```sh
scripts/validate/clang-format.sh
scripts/validate/shellcheck.sh
scripts/validate/strict-libusb-free-compile.sh
MOCHAD_SANITIZERS=disabled scripts/validate/unit-tests.sh
```

Use the manifest `full` profile for TCP, installation, archive, version, and
repository-hygiene checks. Enable sanitizers for release evidence:

```sh
MOCHAD_SANITIZERS=enabled scripts/validate/unit-tests.sh
```

## Prohibited Actions

- Do not change protocol output, USB lifecycle, command parsing, installation,
  or version output as incidental cleanup.
- Do not duplicate bridge device profiles or Docker runtime policy.
- Do not open or transmit through hardware without the workspace hardware
  gates.

Hardware work uses the restricted account, `/run/lock/x10-hardware.lock`,
localhost ports `19000-19999`, house code `D`, reviewed scripts under
`scripts/hardware/`, displayed commands, and explicit human approval.

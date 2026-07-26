# Hardware Validation

Hardware validation is the strongest evidence that `mochad-redux` still works
as a real X10 daemon. It complements CI; it does not replace CI.

Use [the hardware validation guide](../docs/development/hardware-validation.md)
and [the isolated lab setup](../docs/development/hardware-lab-setup.md) as the
step-by-step checklists.

For interactive CM19A evidence collection, use:

```sh
scripts/hardware/cm19a-validation.sh
```

The default run does not transmit. Set `MOCHAD_BIN`, `MOCHAD_LAB_PORT`, or
`EVIDENCE_FILE` to override safe defaults. Transmission requires `--transmit`,
an address under housecode `D`, and the exact interactive approval phrase.

## Minimum Release Evidence

Before a release is described as production-ready, collect:

- CM19A native foreground validation.
- CM19A Docker validation, when Docker packaging is part of the release claim.
- Startup logs from controller detection through listener readiness.
- RF receive output from `nc`.
- Shutdown logs.
- Restart result.

## Preferred Evidence

When hardware is available, also collect:

- CM19A native systemd validation.
- CM15A native foreground validation.
- CM15A Docker validation.
- CM15A native systemd validation.

## Recording Results

Hardware evidence may live in:

- A GitHub hardware-validation issue.
- A release pull request.
- A committed `validation/releases/vX.Y.Z.md` file.

Do not include private hostnames, private paths, credentials, or unnecessary USB
serial details. Keep only the details needed to reproduce the result.

## Waivers

If a release ships without CM15A evidence, say so plainly in the release
evidence and release notes. Avoid implying hardware coverage that was not
actually tested.

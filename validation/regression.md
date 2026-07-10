# Regression Validation

Regression validation protects existing `mochad` behavior. The goal is to catch
accidental changes to startup, TCP compatibility, command parsing, RF receive,
USB ownership, and shutdown behavior.

## Compatibility-Sensitive Areas

Validate these areas when preparing a release or changing nearby code:

- Default TCP listener: `0.0.0.0:1099`.
- Auxiliary listeners: `1100` and `1101` enabled by default.
- Optional XML/OpenRemote disable flags do not affect the main listener.
- Explicit IPv6 bind through `--bind ::`.
- Existing line-oriented TCP output.
- Existing `rf`, `pl`, `rfsec`, and `st` command behavior.
- Diagnostic commands `hello`, `capabilities`, `health`, `clients`, and
  `version` return newline-delimited single-line JSON on the main listener.
- The Flash XMLSocket-compatible listener on port `1100` remains legacy-only:
  it uses NUL-delimited event framing and does not provide structured XML.
- CM19A RF receive path.
- CM15A RF/power-line path when hardware is available.
- USB interface claim, detach, release, and kernel-driver reattach behavior.
- Clean shutdown and restart.

## Manual Smoke Checks

For command-line smoke validation after a native build:

```sh
scripts/validate/native-smoke-test.sh
```

For Docker packaging smoke validation:

```sh
scripts/validate/docker-smoke-test.sh
```

Manual TCP checks can also be run directly:

```sh
./mochad -d
nc localhost 1099
```

While `nc` is connected, press known X10 remote buttons and record the output.

For status:

```sh
printf 'st\n' | nc localhost 1099
```

For diagnostics:

```sh
printf 'hello\n' | nc localhost 1099
printf 'health\n' | nc localhost 1099
printf 'clients\n' | nc localhost 1099
```

For explicit IPv6 listener validation:

```sh
./mochad -d --bind ::
nc -6 ::1 1099
```

## Evidence

Record:

- Exact command used.
- Expected behavior.
- Actual behavior.
- Relevant logs.
- Whether the test used CM19A, CM15A, or no hardware.

If a regression check cannot be performed, document why rather than leaving it
ambiguous.

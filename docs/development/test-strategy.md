# Test Strategy

`mochad-redux` owns C behavior, legacy TCP compatibility, USB safety, native
installation contracts, and controller hardware evidence. `mochad-docker`
owns image assembly and runtime permissions. `mochad-mqtt-bridge` owns MQTT,
Home Assistant discovery, and bridge orchestration.

## Pull Request Checks

Normal pull requests retain the established check names until replacement
workflows have run successfully on `develop` and branch protection is updated.

| Check | Coverage |
| --- | --- |
| C format and shell safety | ClangFormat and ShellCheck |
| Ubuntu LTS | strict compile, sanitizer unit tests, TCP diagnostics, full native build, runtime version |
| Ubuntu Latest | supported toolchain compatibility |
| Debian Stable | distro compatibility |
| Raspberry Pi Cross Compile | ARM compiler compatibility |

The unit runner accepts:

```sh
MOCHAD_SANITIZERS=disabled scripts/validate/unit-tests.sh
MOCHAD_SANITIZERS=enabled scripts/validate/unit-tests.sh
```

Fast local iteration may disable sanitizers. Release evidence enables them.

## Focused Tests

| Area | Primary evidence |
| --- | --- |
| Configuration precedence and validation | `tests/unit/test_config.c` |
| Diagnostic JSON and bounds | `tests/unit/test_diagnostics.c` |
| CM19A RF encoding | `tests/unit/test_encode_rf.c` |
| Legacy native/XML framing | `tests/golden/test_mochad_event.c` |
| Partial writes and `EINTR` | `tests/unit/test_socket_io.c` |
| TCP diagnostic framing | `tests/integration/test_tcp_diagnostics.c` |
| USB endpoint selection | `tests/unit/test_usb_endpoint_selection.c` |
| USB transmit queue lifecycle | `tests/unit/test_x10_write.c` |

These tests preserve deterministic coverage for prior high-severity socket,
endpoint-selection, and transmit-lifecycle findings.

## Release-Only Evidence

The manually dispatched `CI Release Evidence` workflow owns:

- sanitizer execution;
- Ubuntu native and source-archive validation;
- Debian Stable build;
- Raspberry Pi cross compile;
- native setup contract;
- optional exact-SHA `mochad-docker` packaging validation;
- machine-readable status vocabulary;
- explicit hardware-required records.

Real CM19A/CM15A checks are never automatic pull-request jobs. Use
[Hardware Lab Setup](hardware-lab-setup.md) and record only `PASS`, `FAIL`,
`NOT RUN`, `NOT APPLICABLE`, or `HARDWARE REQUIRED`.

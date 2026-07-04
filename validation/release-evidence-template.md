# Release Evidence: vX.Y.Z

## Release Candidate

- Release:
- Commit SHA:
- Branch:
- Date:
- Maintainer:
- Upstream base:

## Summary

Briefly describe the release scope. Note whether this release changes behavior,
build/release process, documentation, or diagnostics.

## Compiler Validation

Record the exact commands and results.

| Check | Environment | Command | Result | Notes |
| --- | --- | --- | --- | --- |
| libusb-free strict build |  | `sh tools/compile_without_libusb.sh --strict --asan --ubsan` |  |  |
| cppcheck |  | `sh tools/compile_without_libusb.sh --cppcheck` |  |  |
| full libusb build |  | `./autogen.sh && ./configure && make` |  |  |
| Ubuntu LTS CI | GitHub Actions | CI workflow |  |  |
| Ubuntu Latest CI | GitHub Actions | CI workflow |  |  |
| Debian Stable CI | GitHub Actions | CI workflow |  |  |
| Raspberry Pi cross compile | GitHub Actions | CI workflow |  |  |

## Static Analysis

| Tool | Command | Result | Notes |
| --- | --- | --- | --- |
| cppcheck | `sh tools/compile_without_libusb.sh --cppcheck` |  |  |
| clang-tidy | `sh tools/compile_without_libusb.sh --clang-tidy` |  | Optional when available |
| clang-format check | `sh tools/compile_without_libusb.sh --clang-format-check` |  | Optional when available |

## Runtime Validation

| Area | Runtime | Result | Evidence |
| --- | --- | --- | --- |
| Startup logs | native foreground |  |  |
| TCP listener on 1099 | native foreground |  |  |
| IPv4 default bind | native foreground |  |  |
| Explicit IPv6 bind | native foreground |  |  |
| Shutdown logs | native foreground |  |  |
| Docker health check | Docker |  |  |

## Hardware Validation

| Controller | Runtime | Result | Evidence |
| --- | --- | --- | --- |
| CM19A | Docker |  |  |
| CM19A | Native foreground |  |  |
| CM19A | Native systemd |  |  |
| CM15A | Docker |  |  |
| CM15A | Native foreground |  |  |
| CM15A | Native systemd |  |  |

## Regression Validation

Record compatibility-sensitive behavior that should not regress.

| Behavior | Command/Input | Expected Result | Actual Result |
| --- | --- | --- | --- |
| TCP connection accepts client | `nc localhost 1099` | connection succeeds |  |
| RF event appears in TCP stream | press X10 remote button | `Rx RF ...` line appears |  |
| Clean shutdown | `Ctrl+C` foreground process | shutdown logs complete |  |
| Restart releases USB and TCP resources | restart after shutdown | controller and port are available |  |

## Known Limitations

- 

## Release Decision

- [ ] CI passed.
- [ ] Local validation commands passed.
- [ ] Hardware evidence is linked or explicitly waived.
- [ ] Known limitations are documented.
- [ ] Generated-artifact policy was followed.
- [ ] Release notes mention validation evidence.

Decision:

- [ ] Release approved.
- [ ] Release blocked.

Reason:

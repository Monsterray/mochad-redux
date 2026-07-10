# Supported Platforms

`mochad-redux` is Linux-first software for X10 CM15A and CM19A USB
controllers. Support claims should be based on build evidence, CI evidence, and
hardware validation evidence.

## Support Levels

| Level | Meaning |
| --- | --- |
| Supported | Expected to work and validated for releases. |
| Best effort | Intended to work, but validation may be incomplete. |
| Build-only | Useful for compile checks, not a runtime support claim. |
| Legacy reference | Kept for compatibility or historical context. |

## Runtime Platforms

| Platform | Level | Evidence Required |
| --- | --- | --- |
| Linux x86_64 | Supported | CI build plus release validation when hardware is available. |
| Linux arm64 / Raspberry Pi OS | Supported | CI/cross-compile plus hardware validation when available. |
| Debian Stable | Supported | CI build. |
| Ubuntu LTS | Supported | CI build. |
| Ubuntu Latest | Supported | CI build. |
| Docker on Linux | Supported | Docker packaging validation plus USB passthrough notes. |
| macOS | Build-only | libusb-free compile checks only. |
| Windows | Not supported | No native runtime support planned. |

## Hardware

| Hardware | Level | Notes |
| --- | --- | --- |
| CM19A / CM19Pro | Supported after validation | RF path. Required release evidence for production claims. |
| CM15A / CM15Pro | Best effort until validated | RF and power-line path. Record clearly when not tested. |

## Networking

| Mode | Level | Notes |
| --- | --- | --- |
| IPv4 default bind `0.0.0.0` | Supported | Default behavior. |
| IPv4 loopback `127.0.0.1` | Supported | Runtime `--bind` option. |
| Explicit IPv6 bind `::` / `::1` | Supported with host policy caveat | Dual-stack may be blocked by kernel, sysctl, or Docker policy. |

## Legacy Support Files

The `apps/`, `cgi/`, `hotplug2/`, `udev/`, and `systemd/` folders are described
in [support-files.md](support-files.md). They are not all equal support
surfaces:

- `systemd/` and `udev/` are active Linux deployment support files.
- `hotplug2/` is legacy embedded Linux support.
- `apps/` and `cgi/` are historical examples and should not be treated as
  maintained production clients without review.

## Updating This Matrix

Only upgrade a support level when release evidence exists. If evidence is lost
or stale, keep the support claim conservative.

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
| macOS 13 x86_64 | Best effort | Native libusb build and CM19A foreground receive/transmit validation at `fde2bb21`; other macOS versions and Apple silicon remain unvalidated. |
| Windows | Not supported | No native runtime support planned. |

## Historical Upstream Evidence

The earlier fork line recorded successful tests on these exact systems. These
records are historical evidence, not current release certification:

- x86_64 GNU/Linux: Mint 20.1, Ubuntu 20.04 LTS, Linux 5.4.0-124.
- aarch64 GNU/Linux: Armbian 22.05.3, Ubuntu 22.04.1 LTS, Linux
  5.10.123-meson64.
- aarch64 GNU/Linux: Raspberry Pi OS 2022-04-04, Debian 11.4, Linux
  5.15.32-v8+.
- armv6l GNU/Linux: Raspberry Pi OS 2024-01-25, Debian 12.1, Linux 6.1.0.

## Hardware

| Hardware | Level | Notes |
| --- | --- | --- |
| CM19A / CM19Pro | Supported after validation | RF path. Required release evidence for production claims. |
| CM15A / CM15Pro | Best effort until validated | RF and power-line path. Record clearly when not tested. |

The recorded macOS CM19A run is in
[macOS 13 CM19A validation](../../validation/hardware/macos-13-cm19a-2026-08-22.md).
It demonstrates one exact host, controller, and commit; it is not evidence for
CM15A, Apple silicon, launchd integration, or every supported macOS release.

CM19A hotplug removal and arrival are logged, but automatic recovery is not
currently supported. Restart `mochad` after reconnecting the controller. The
`usb_connected` diagnostic may remain stale after removal; `transfers_ready`
must also be true before treating the controller as usable.

## Networking

| Mode | Level | Notes |
| --- | --- | --- |
| IPv4 default bind `0.0.0.0` | Supported | Default behavior. |
| IPv4 loopback `127.0.0.1` | Supported | Runtime `--bind` option. |
| Explicit IPv6 bind `::` / `::1` | Supported with host policy caveat | Dual-stack may be blocked by kernel, sysctl, or Docker policy. |

## Legacy Support Files

The `contrib/`, `packaging/`, and `docs/research/legacy-*` folders are
described in [legacy support files](../development/legacy-support-files.md).
They are not equal support surfaces:

- `packaging/linux/systemd/` and `packaging/linux/udev/` provide inactive
  templates rendered by the explicit native setup tool; package installation
  does not activate them.
- `packaging/openwrt/` is legacy embedded Linux support.
- `contrib/apps/` contains unsupported examples.
- `docs/research/legacy-*` contains non-authoritative historical artifacts.
- `docs/research/legacy-cgi/` is a withdrawn Perl CGI interface. A security
  review found unauthenticated, remotely reachable defects. It remains tracked
  as historical source, but it is not installed or supported and must not be
  deployed.

## Updating This Matrix

Only upgrade a support level when release evidence exists. If evidence is lost
or stale, keep the support claim conservative.

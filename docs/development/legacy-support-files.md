# Support Files

This repository keeps several legacy support directories from upstream
`mochad`. They are useful references, but they do not all represent modern
recommended deployment paths.

## contrib/apps

`contrib/apps/` contains example clients and small integration scripts:

- `simplemon.pl` and `mochamon.pl` monitor one or more `mochad` TCP streams.
- `rfsectopl3.pl` and `bash.sh` translate RF security events into X10 power-line
  commands.

These files are historical examples. Several contain site-specific hostnames,
hardcoded IPv4 addresses, or assumptions about local tools such as `flite`.
They should not be treated as maintained production clients without review.

The Python 2/Tk `x10-tk.py` demo and the process-killing/FIFO `mochad.scr`
helper are retained under `docs/research/legacy-apps/`. They are historical
evidence, not supported programs, and must not be executed without review.

## cgi

`cgi/` contains an old Perl CGI interface for controlling X10 devices and
displaying status. It includes:

- `x10.pl`, the web entrypoint.
- `netcat.pl`, a small TCP command helper.
- `getsensors.pl`, a status-table renderer.
- `cgi-lib.pl`, a bundled third-party CGI parsing library from the 1990s.

This folder is legacy reference material. It is IPv4-only, embeds local network
addresses, and predates modern web security expectations. Do not expose it on a
network without a full security review. For current deployments, prefer the
daemon TCP interface or the separate MQTT bridge project.

## OpenWrt

`packaging/openwrt/` contains OpenWrt-era hotplug/init scripts:

- `20-usb-x10` starts `/etc/init.d/mochad` when a CM15A or CM19A appears.
- `mochad` is an `/etc/rc.common` init script using `start-stop-daemon`.

These are useful for older embedded Linux systems that do not use systemd. The
USB match should be kept conservative, and future changes should avoid adding
distribution-specific behavior here unless it is tested on that target.

## udev

`packaging/linux/udev/` contains the maintained USB rule template for Linux
systems. Historical fixed-group and systemd-activation rules are retained
under `docs/research/legacy-packaging/udev/` for provenance:

- `91-usb-x10-controllers.rules.in` is rendered by `mochad-redux-setup`; it
  assigns `root:x10` ownership and `0660` mode to supported X10 USB nodes.
- The legacy rule files remain source references. `make install` places only
  inactive templates beneath the configured prefix.

The setup tool performs host integration explicitly, so udev never launches
the daemon directly.

## systemd

`packaging/linux/systemd/mochad.service.in` is rendered into the existing
`mochad.service` unit name by the setup tool. It resolves the installed binary
path and uses user `mochad`, group `mochad`, supplementary group `x10`, and
`UMask=0022` by default. The historical fixed-path unit is retained under
`docs/research/legacy-packaging/systemd/` and is not authoritative packaging.

Potential future hardening includes explicit device dependencies and optional
service sandboxing. Those changes should be tested with real CM15A/CM19A
hardware before becoming defaults, because USB device access and kernel-driver
detaching can be sensitive to service permissions.

## Authority Summary

| Area | Authoritative path | Historical or unsupported path |
| --- | --- | --- |
| Native systemd | `packaging/linux/systemd/mochad.service.in` | `docs/research/legacy-packaging/systemd/` |
| Native udev | `packaging/linux/udev/91-usb-x10-controllers.rules.in` | `docs/research/legacy-packaging/udev/` |
| Native setup | `scripts/setup/mochad-redux-setup.in` | None |
| OpenWrt | `packaging/openwrt/` | Legacy compatibility surface |
| Example clients | `contrib/apps/` | Unsupported examples |
| Obsolete applications | None | `docs/research/legacy-apps/` |
| CGI | Undecided | Root `cgi/`, retained pending provenance/support decision |

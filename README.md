# mochad-redux

`mochad` is a Linux TCP gateway daemon for the X10 CM15A RF/power-line and
CM19A RF controllers.

`mochad-redux` is a maintenance-focused fork intended to preserve upstream
behavior while improving build hygiene, diagnostics, safety, and long-term
maintainability.

## Project Background

This fork descends from the [Neil Cherry linuxha fork](https://github.com/linuxha/mochad)
and the original `mochad` 0.1.x releases by mmauka. The original code needed
updates to compile and install cleanly on modern Linux distributions using
`systemd`.

Project lineage:
[bjonica/mochad](https://github.com/bjonica/mochad) to
[linuxha/mochad](https://github.com/linuxha/mochad) to
[sigmdel/mochad](https://github.com/sigmdel/mochad) to `mochad-redux`.

The earlier fork this work builds from was tested on:

- x86_64 GNU/Linux: Mint 20.1, Ubuntu 20.04 LTS, Linux 5.4.0-124
- aarch64 GNU/Linux: Armbian 22.05.3, Ubuntu 22.04.1 LTS, Linux 5.10.123-meson64
- aarch64 GNU/Linux: Raspberry Pi OS 2022-04-04, Debian 11.4, Linux 5.15.32-v8+
- armv6l GNU/Linux: Raspberry Pi OS 2024-01-25, Debian 12.1, Linux 6.1.0

## Maintenance Goals

The first maintenance milestone is intentionally conservative:

- Keep protocol behavior compatible with existing `mochad` users.
- Improve compiler warning coverage and static analysis.
- Fix concrete safety and ownership issues in small reviewable changes.
- Improve diagnostics without changing the TCP protocol layout.
- Treat the `v0.4.x` line as runtime hardening and observability work:
  startup, shutdown, listener, USB, and client lifecycle logs come before new
  protocol features.
- Keep future rebases from upstream manageable.

Broader protocol, USB, TCP, and state separation work is future work.
Engineering principles are documented in [DESIGN.md](DESIGN.md).

## Source Notes

Older `mochad` sources could fail to link with errors similar to:

```text
/usr/bin/ld: decode.o: global.h: multiple definition of `RfToRf16'
/usr/bin/ld: decode.o: global.h: multiple definition of `RfToPl16'
/usr/bin/ld: decode.o: global.h: multiple definition of `PollTimeOut'
/usr/bin/ld: decode.o: global.h: multiple definition of `Cm19a'
```

That issue was addressed by declaring shared variables as `extern` in
`global.h` and defining storage in `global.c`.

The `systemd` and `udev` directories were restored from the original
[mochad-0.1.17](https://sourceforge.net/projects/mochad/files/) release so the
service and device rules install correctly on modern systems.

Legacy examples and service integration files are documented in
[docs/support-files.md](docs/support-files.md).

## Installation

Install the userspace USB development library:

```sh
sudo apt install libusb-1.0-0-dev
```

On Raspberry Pi systems, `autoconf` may also be required:

```sh
sudo apt install autoconf
```

`netcat` is useful for manual testing:

```sh
sudo apt install netcat-openbsd
```

Clone the source:

```sh
git clone https://github.com/StoaferP/mochad-redux.git
cd mochad-redux
```

Build the project:

```sh
chmod +x autogen.sh
./autogen.sh
make
```

Install the package:

```sh
sudo make install
```

Expected installed files include:

```text
/usr/local/bin/mochad
/etc/udev/rules.d/91-usb-x10-controllers.rules
/etc/systemd/system/mochad.service
```

The service remains inactive until a supported CM15A or CM19A controller is
connected. The installed `udev` rules handle service activation.

## Testing

Connect a CM15A or CM19A to USB, then connect to `mochad` with `netcat` and
press buttons on an X10 RF remote:

```sh
nc localhost 1099
```

Example output:

```text
02/03 19:27:40 Rx RF HouseUnit: K4 Func: On
02/03 19:27:44 Rx RF HouseUnit: K6 Func: Off
02/03 19:27:46 Rx RF House: K Func: Dim
```

Use `Ctrl+C` to close `netcat`.

For build-only checks that do not require `libusb`, use:

```sh
sh tools/compile_without_libusb.sh --strict
```

Optional analyzer modes are documented in [MAINTAINING.md](MAINTAINING.md).
Real controller testing is documented in
[docs/hardware-validation.md](docs/hardware-validation.md).

## Runtime Options

By default, `mochad-redux` listens on `0.0.0.0:1099`, with auxiliary legacy
listeners on `1100` and `1101`.

```sh
mochad -d \
  --bind 0.0.0.0 \
  --port 1099 \
  --xml-port 1100 \
  --openremote-port 1101
```

Ports must be distinct TCP ports from `1` to `65535`. Invalid bind addresses or
ports fail at startup with a clear error.

## IPv6

IPv6 is configured at runtime with `--bind`; no source edit or rebuild is
required.

Common bind addresses:

```text
0.0.0.0    all IPv4 interfaces, default
127.0.0.1  IPv4 loopback only
::         all IPv6 interfaces, with dual-stack IPv4-mapped connections when
           the operating system allows them
::1        IPv6 loopback only
```

Examples:

```sh
mochad -d --bind ::
mochad -d --bind ::1 --port 1099 --xml-port 1100 --openremote-port 1101
```

When bound to `::`, `mochad-redux` asks the operating system for a dual-stack
listener by disabling `IPV6_V6ONLY`. Some systems may still restrict this by
sysctl or kernel policy; in that case use an explicit IPv4 bind address for
IPv4 clients or an explicit IPv6 bind address for IPv6 clients.

Startup logs show the chosen address family, bind address, port, and
dual-stack result for each listener:

```text
[TCP] listener ready name=main address=:: port=1099 family=ipv6 dual_stack=enabled
[TCP] listener ready name=xml address=:: port=1100 family=ipv6 dual_stack=enabled
[TCP] listener ready name=openremote address=:: port=1101 family=ipv6 dual_stack=enabled
```

If the host or container runtime blocks IPv4-mapped IPv6 sockets, the log may
show `dual_stack=failed`. In that case, keep the default `--bind 0.0.0.0` for
IPv4-only service or explicitly use `--bind ::` for IPv6-only validation.

## Troubleshooting

If no RF activity appears when pressing remote buttons, check the service:

```sh
systemctl status mochad.service
```

If the output contains this error:

```text
usb_claim_interface failed -6
```

check whether the `ati_remote` kernel module is loaded:

```sh
lsmod | grep ati_remote
```

The ATI All-In-Wonder Lola remote uses the same `0x0bc7:0x002` USB ID as the
CM19A, so Linux may load `ati_remote` for the controller. Blacklist the module
and reboot:

```sh
echo "blacklist ati_remote" | sudo tee /usr/lib/modprobe.d/ati-remote-blacklist.conf
```

Trying to unload the module with `sudo modprobe -r ati_remote` may not be
enough if the device is already claimed.

## More Information

- [Original README](README)
- [Mochad on Recent Linux Distributions](https://sigmdel.ca/michel/ha/domoticz/mochad_on_recent_linux_distro_en.html)
- [French installation notes](https://sigmdel.ca/michel/ha/domoticz/mochad_on_recent_linux_distro_fr.html)
- [Andreas's systemd unit discussion](https://sourceforge.net/p/mochad/discussion/1320002/thread/764dd1ce44/#76e9)
- [Steve Porter's mochad-0.1.21 discussion](https://sourceforge.net/p/mochad/discussion/1320002/thread/9e758b6afc/)
- [clangen/mochad](https://github.com/clangen/mochad)

## License

GNU General Public License version 3.0 or later (GPL-3.0-or-later), according
to the source file headers and the
[original project page on SourceForge](https://sourceforge.net/projects/mochad/).
See [LICENSE.md](LICENSE.md).

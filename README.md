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
Engineering principles are documented in
[the design guide](docs/architecture/design.md).

## Versioning

The root [VERSION](VERSION) file is the maintained project version source.
Version files use plain semantic versions such as `0.4.0`, `0.5.0-dev`, or
`0.5.0-rc1`; Git tags add the leading `v`. The upstream baseline remains the
separate identity `mochad 0.1.18`.

See [compatibility and versioning](docs/architecture/compatibility.md) for the maintained
version mapping and the small, manual-friendly release preparation workflow.

## Source Notes

Older `mochad` sources could fail to link with errors similar to:

```text
/usr/bin/ld: decode.o: global.h: multiple definition of `RfToRf16'
/usr/bin/ld: decode.o: global.h: multiple definition of `RfToPl16'
/usr/bin/ld: decode.o: global.h: multiple definition of `PollTimeOut'
/usr/bin/ld: decode.o: global.h: multiple definition of `Cm19a'
```

That issue was addressed by declaring shared variables as `extern` in
`src/core/global.h` and defining storage in `src/core/global.c`.

The `systemd` and `udev` directories were restored from the original
[mochad-0.1.17](https://sourceforge.net/projects/mochad/files/) release so the
service and device rules install correctly on modern systems.

Legacy examples and service integration files are documented in
[legacy support-file documentation](docs/development/legacy-support-files.md).

## Repository Layout

Maintained code and support files are grouped by ownership:

- `src/` contains the daemon, configuration, network, USB, and X10 code.
- `tests/` contains unit, golden-output, integration, and test-support files.
- `packaging/` contains maintained Linux templates and retained OpenWrt files.
- `scripts/` contains build, formatting, setup, release, hardware, and
  validation commands.
- `contrib/` contains unsupported but reviewable client examples.
- `docs/research/` contains historical, superseded, or provenance-sensitive
  material that must not be treated as current installation guidance.
- `validation/` contains release evidence policy and records.

See the [documentation index](docs/README.md), the
[architecture guide](docs/architecture/architecture.md), and the
[repository layout record](docs/development/repository-layout-proposal.md) for
ownership and compatibility details.

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
git clone https://github.com/Monsterray/mochad-redux.git
cd mochad-redux
```

Git checkouts need Autotools before configuration:

```sh
./autogen.sh
./configure
make
```

Release archives already include `configure`, so use:

```sh
./configure
make
```

## Native Linux Install

The recommended bare-metal path is explicit and safe to repeat:

```sh
sudo make install
sudo mochad-redux-setup install --enable-now
```

The Autotools install target only copies files into the configured prefix and
honors `DESTDIR`. It installs inactive templates, documentation, licenses, and
the `mochad-redux-setup` administration tool. It never creates accounts,
modifies live `/etc`, calls `systemctl` or `udevadm`, starts services, or opens
the controller.

For a source-tree convenience path after a successful build:

```sh
sudo ./scripts/setup/install-native.sh --enable-now
```

`mochad-redux-setup` manages the default `mochad` user and group, the `x10`
supplementary USB group, a `root:x10` / `0660` udev rule, and the existing
`mochad.service` unit name. It records managed integration files and refuses to
replace local edits unless `--force` is given. It preserves
`/etc/mochad-redux/mochad.conf` during upgrades and does not restart an active
service unless `--restart` or `--enable-now` is explicit.

Preview native integration first with:

```sh
sudo mochad-redux-setup install --dry-run
```

## Package Staging

Package builders should stage only files and perform no host integration:

```sh
make DESTDIR="$PWD/stage" install
```

## Development And Containers

Run the daemon directly during development or use an isolated prefix. Never run
`mochad-redux-setup` inside `mochad-docker`; container permissions are managed
by that project instead. Removal and rollback guidance is in
[native installation rollback](docs/installation/native-install-rollback.md).

## Beta Testing

Version 0.4.0 is a cautious public beta. Use a tagged beta release or exact
full Git SHA, not a moving branch. Source-level validation has passed; CM19A,
CM15A, and module behavior still require physical evidence.

See [the beta-status guide](docs/release/beta-status.md), the
[hardware validation guide](docs/development/hardware-validation.md), the
[isolated hardware-lab setup](docs/development/hardware-lab-setup.md), and the
Beta test report issue form before testing a non-critical controller or module.

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
sh scripts/build/compile-without-libusb.sh --strict
```

To syntax-check the USB-facing source on a development machine without real
libusb headers installed, use the checked-in development stub:

```sh
scripts/validate/libusb-stub-syntax-check.sh
```

This does not replace a real Linux/libusb build. It only keeps maintainers from
missing ordinary C syntax or include errors in `src/core/mochad.c` while
working on machines such as macOS.

To run sanitizer-backed unit tests and validate diagnostic JSON over a loopback
TCP socket:

```sh
scripts/validate/unit-tests.sh
scripts/validate/tcp-diagnostics-smoke-test.sh
scripts/validate/version-consistency.sh
```

To remove ignored build artifacts before validating, use:

```sh
scripts/validate/clean-build-test.sh
```

In environments without libusb headers, run the clean libusb-free validation:

```sh
scripts/validate/clean-build-test.sh --libusb-free-only
```

Optional analyzer modes are documented in
[the maintainer guide](docs/development/maintaining.md).
Real controller testing is documented in
[the hardware validation guide](docs/development/hardware-validation.md).
Release evidence is documented in [validation/README.md](validation/README.md).
Supported platform expectations are documented in
[supported platforms](docs/installation/supported-platforms.md).

## Runtime Options

By default, `mochad-redux` listens on `0.0.0.0:1099`, with auxiliary legacy
listeners on `1100` and `1101`.

```sh
mochad -d \
  --bind 0.0.0.0 \
  --port 1099 \
  --enable-xml \
  --xml-port 1100 \
  --enable-openremote \
  --openremote-port 1101
```

The XML and OpenRemote listeners are enabled by default for backward
compatibility. Use `--disable-xml` or `--disable-openremote` to turn off either
auxiliary listener while keeping the main TCP listener on. Enabled listener
ports must be distinct TCP ports from `1` to `65535`. Invalid bind addresses or
ports fail at startup with a clear error.

A future generic JSON-RPC API is documented in
[the JSON API design](docs/protocol/json-api.md). It is not implemented in the current
runtime. The proposed listener is optional, disabled by default while
experimental, and intended to use port `1102` without changing the existing
`1099`, `1100`, or `1101` listener contracts.

Configuration is applied in a predictable order:

1. compiled defaults
2. optional config file from `MOCHAD_CONFIG` or `--config FILE`
3. environment variables
4. command-line options

Useful configuration commands:

```sh
mochad --check-config
mochad --print-config
```

Supported environment variables:

```text
MOCHAD_CONFIG
MOCHAD_BIND or MOCHAD_BIND_ADDRESS
MOCHAD_PORT or MOCHAD_SERVER_PORT
MOCHAD_XML_ENABLED
MOCHAD_XML_PORT
MOCHAD_OPENREMOTE_ENABLED
MOCHAD_OPENREMOTE_PORT
MOCHAD_FOREGROUND
MOCHAD_RAW_DATA
MOCHAD_DUAL_STACK
MOCHAD_LOG_LEVEL
```

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

## TCP Diagnostics

The main TCP listener accepts backward-compatible diagnostic commands. These
commands return one JSON object per line and do not change legacy command
behavior.

```sh
printf 'hello\n' | nc localhost 1099
printf 'capabilities\n' | nc localhost 1099
printf 'health\n' | nc localhost 1099
printf 'clients\n' | nc localhost 1099
printf 'config\n' | nc localhost 1099
printf 'version\n' | nc localhost 1099
```

These commands are intended for health checks, MQTT bridge integration, and
release validation.

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

- [Source lineage and upstream baseline](docs/research/source-lineage.md)
- [Mochad on Recent Linux Distributions](https://sigmdel.ca/michel/ha/domoticz/mochad_on_recent_linux_distro_en.html)
- [French installation notes](https://sigmdel.ca/michel/ha/domoticz/mochad_on_recent_linux_distro_fr.html)
- [Andreas's systemd unit discussion](https://sourceforge.net/p/mochad/discussion/1320002/thread/764dd1ce44/#76e9)
- [Steve Porter's mochad-0.1.21 discussion](https://sourceforge.net/p/mochad/discussion/1320002/thread/9e758b6afc/)
- [clangen/mochad](https://github.com/clangen/mochad)

## License

GNU General Public License version 3.0 or later (GPL-3.0-or-later). See
[LICENSE.md](LICENSE.md), [COPYING](COPYING), [NOTICE](NOTICE), and
[docs/research/source-lineage.md](docs/research/source-lineage.md).

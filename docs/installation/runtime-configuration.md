# Runtime Configuration

This reference preserves the complete daemon configuration and troubleshooting
details while the root README remains a concise installation entry point.

## Listener Options

By default, `mochad-redux` listens on `0.0.0.0:1099`, with auxiliary legacy
listeners on ports `1100` and `1101`.

```sh
mochad -d \
  --bind 0.0.0.0 \
  --port 1099 \
  --enable-xml \
  --xml-port 1100 \
  --enable-openremote \
  --openremote-port 1101
```

The XMLSocket and OpenRemote listeners are enabled by default for backward
compatibility. Use `--disable-xml` or `--disable-openremote` to turn off either
listener. Enabled listener ports must be distinct values from `1` to `65535`.
Invalid bind addresses or ports fail before USB initialization.

## Configuration Sources

Configuration is applied in this order:

1. Compiled defaults.
2. Optional file from `MOCHAD_CONFIG` or `--config FILE`.
3. Environment variables.
4. Command-line options.

Inspect configuration without opening a USB controller:

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

IPv6 is configured with `--bind`; no rebuild is required.

```text
0.0.0.0    all IPv4 interfaces, default
127.0.0.1  IPv4 loopback only
::         all IPv6 interfaces, with IPv4-mapped dual stack when allowed
::1        IPv6 loopback only
```

Examples:

```sh
mochad -d --bind ::
mochad -d --bind ::1 --port 1099 --xml-port 1100 --openremote-port 1101
```

When bound to `::`, the daemon requests dual-stack operation by disabling
`IPV6_V6ONLY`. The kernel, sysctl policy, or container runtime may still reject
that request. Startup logs report the address family, bind address, port, and
dual-stack result for every listener:

```text
[TCP] listener ready name=main address=:: port=1099 family=ipv6 dual_stack=enabled
[TCP] listener ready name=xml address=:: port=1100 family=ipv6 dual_stack=enabled
[TCP] listener ready name=openremote address=:: port=1101 family=ipv6 dual_stack=enabled
```

If the log reports `dual_stack=failed`, use `0.0.0.0` for IPv4 service or
validate explicit IPv6 connectivity with `::`.

## TCP Diagnostics

The main listener accepts backward-compatible, read-only diagnostics. Each
returns one JSON object followed by a newline:

```sh
printf 'hello\n' | nc localhost 1099
printf 'capabilities\n' | nc localhost 1099
printf 'health\n' | nc localhost 1099
printf 'clients\n' | nc localhost 1099
printf 'config\n' | nc localhost 1099
printf 'version\n' | nc localhost 1099
printf 'evidence\n' | nc localhost 1099
```

These commands are used by health checks, the MQTT bridge, and release
validation. The future generic JSON-RPC design is documented in
[JSON API design](../protocol/json-api.md); port `1102` is not implemented.
The bounded `evidence` response is documented in
[Transport evidence](../operations/transport-evidence.md).

## USB Troubleshooting

If RF activity is absent, inspect the native service:

```sh
systemctl status mochad.service
```

If controller startup reports `usb_claim_interface failed -6`, check whether
the `ati_remote` kernel module claimed the device:

```sh
lsmod | grep ati_remote
```

The ATI All-In-Wonder Lola remote uses the same `0x0bc7:0x0002` USB identity
as the CM19A, so Linux may load `ati_remote`. Blacklist it and reboot:

```sh
echo "blacklist ati_remote" | \
  sudo tee /usr/lib/modprobe.d/ati-remote-blacklist.conf
```

Unloading the module after the controller is already claimed may not be
sufficient.

## Native Setup and Rollback

The installed `mochad-redux-setup` tool owns host integration. It defaults to
service user and group `mochad`, supplementary USB group `x10`, USB mode
`0660`, and the established `mochad.service` unit name.

It tracks managed files, preserves the active configuration during upgrades,
refuses unmanaged or locally modified files without `--force`, and never
restarts an active service unless `--restart` or `--enable-now` is explicit.
Normal removal does not delete users, groups, or configuration.

See [native rollback](native-install-rollback.md) for the complete managed
cleanup workflow.

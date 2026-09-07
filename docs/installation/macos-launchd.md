# macOS launchd Integration

Run `mochad` as a system-wide launchd daemon on macOS so it starts at boot
and restarts after failures. This is the macOS counterpart of the Linux
systemd unit: launchd supervises a foreground (`-d`) process directly, so
the plist never uses `--background`.

This path is an example for the Best-effort macOS platform described in
[supported platforms](supported-platforms.md). It does not change the
support level on its own; launchd supervision with attached hardware remains
to be recorded as release evidence.

## Prerequisites

- A `mochad` binary built from this source on macOS (see the
  [macOS 13 CM19A validation](../../validation/hardware/macos-13-cm19a-2026-08-22.md)
  for the tested toolchain: Apple Clang with Homebrew `libusb`).
- A CM15A (`0bc7:0001`) or CM19A (`0bc7:0002`) controller. Only the CM19A
  has hardware evidence on macOS.
- Administrator access for `/Library/LaunchDaemons`.

macOS has no `udev` group-permission step: USB access goes through
IOKit/libusb. If the daemon reports the controller is unavailable, check
that no other application holds it and that the USB device is visible in
`system_profiler SPUSBDataType`.

## Install

Render the inactive template shipped under `packaging/` (installed
copy-only to `<prefix>/share/mochad-redux/templates/` by `make install`;
nothing is activated by installation itself):

```sh
BINARY=/usr/local/bin/mochad
sed "s|@MOCHAD_BINARY@|$BINARY|g" \
  /usr/local/share/mochad-redux/templates/com.mochad-redux.mochad.plist.in \
  > /tmp/com.mochad-redux.mochad.plist
grep -c '@MOCHAD_BINARY@' /tmp/com.mochad-redux.mochad.plist && echo 'render incomplete'
```

Adjust `BINARY` when the daemon lives elsewhere (for example
`/opt/homebrew/bin/mochad`). Extra daemon flags such as
`--bind 127.0.0.1` can be appended to `ProgramArguments` after `-d`.

Install and bootstrap the rendered plist:

```sh
sudo mkdir -p /var/log/mochad
sudo cp /tmp/com.mochad-redux.mochad.plist /Library/LaunchDaemons/
sudo chown root:wheel /Library/LaunchDaemons/com.mochad-redux.mochad.plist
sudo chmod 644 /Library/LaunchDaemons/com.mochad-redux.mochad.plist
plutil -lint /Library/LaunchDaemons/com.mochad-redux.mochad.plist
sudo launchctl bootstrap system/ /Library/LaunchDaemons/com.mochad-redux.mochad.plist
```

Verify supervision and output:

```sh
sudo launchctl print system/com.mochad-redux.mochad
tail -F /var/log/mochad/mochad.log
printf 'hello\n' | nc 127.0.0.1 1099
```

## Behavior Notes

- `KeepAlive` uses `SuccessfulExit: false`, so a clean SIGTERM shutdown
  stays stopped while a failure exit restarts the daemon. Exiting with
  status 1 when no controller is attached counts as a failure: expect a
  respawn roughly every `ThrottleInterval` (10 seconds) until the
  controller appears. This is intentional for boot-before-device setups.
- Hotplug recovery after unplug/replug has hardware evidence for two
  native foreground cycles on macOS 13 x86_64; the launchd-supervised
  variant has not been recorded separately.
- Do not add `--background` to `ProgramArguments`. A self-detaching
  process escapes launchd supervision and defeats `KeepAlive`.

## Remove

```sh
sudo launchctl bootout system/ /Library/LaunchDaemons/com.mochad-redux.mochad.plist
sudo rm /Library/LaunchDaemons/com.mochad-redux.mochad.plist
```

Log files under `/var/log/mochad/` are left in place and can be removed
manually.

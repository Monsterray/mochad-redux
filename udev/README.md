# udev Rules

`udev` rules to start `mochad`:

  -  `91-usb-x10-controllers.rules` are the `udev` rules for non `systemd` init systems.

  -  `91-usb-x10-controllers.rules-systemd` (renamed `91-usb-x10-controllers.rules` during installation) are for `systemd`.

For modern Linux systems, prefer the systemd rule. It asks systemd to start
`mochad.service` with `--no-block`, so udev does not run the daemon directly or
wait for a long-lived process.

The non-systemd rule is kept for compatibility with older init systems. Avoid
expanding it unless the target system has been tested, because long-running
processes launched directly by udev can be killed by udev or block device
processing.

# udev Rules

`udev` rules for X10 USB controller permissions and service activation:

  -  `91-usb-x10-controllers.rules` are the `udev` rules for non `systemd` init systems.

  -  `91-usb-x10-controllers.rules-systemd` (renamed `91-usb-x10-controllers.rules` during installation) are for `systemd`.

For modern Linux systems, prefer the systemd rule. It assigns X10 USB devices
to group `x10` with mode `0660` and asks systemd to start `mochad.service`.
The service runs as user `mochad` with supplementary group `x10`.

The non-systemd rule only assigns group and mode. It does not run `mochad`
directly because long-running processes launched by udev can be killed by udev
or block device processing.

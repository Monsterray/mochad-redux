# udev Rules

`udev` templates for X10 USB controller permissions:

`91-usb-x10-controllers.rules.in` is an inactive template. After a package
install, `sudo mochad-redux-setup install` renders it into the host udev rules
directory with the selected USB group, `x10` by default, and mode `0660`.
`make install` does not activate a udev rule or invoke `udevadm`.

The rule only assigns group and mode. It does not run `mochad` directly because
long-running processes launched by udev can be killed by udev or block device
processing. Systemd activation is managed separately by the setup tool.


# Old udev rule (99-cm19a.rules)
```
SUBSYSTEM=="usb", ATTR{idVendor}=="0bc7", ATTR{idProduct}=="0002", GROUP="x10", MODE="0660"

blacklist ati_remote
```

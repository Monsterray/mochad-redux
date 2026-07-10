# Native Install Rollback

Use this checklist when removing a native Linux `mochad-redux` installation.

These steps are intentionally manual. The setup helper never removes users or
groups automatically because those accounts may be reused by local policy.

## Stop Service Activation

```sh
sudo systemctl stop mochad.service || true
sudo systemctl disable mochad.service || true
```

## Remove systemd Files

```sh
sudo rm -f /etc/systemd/system/mochad.service
sudo rm -rf /etc/systemd/system/mochad.service.d
sudo systemctl daemon-reload
sudo systemctl reset-failed mochad.service || true
```

## Remove udev Rules

```sh
sudo rm -f /etc/udev/rules.d/91-usb-x10-controllers.rules
sudo udevadm control --reload-rules
sudo udevadm trigger --action=add --subsystem-match=usb --attr-match=idVendor=0bc7 --attr-match=idProduct=0001
sudo udevadm trigger --action=add --subsystem-match=usb --attr-match=idVendor=0bc7 --attr-match=idProduct=0002
sudo udevadm settle
```

Replug CM15A/CM19A controllers after removing the rule if permissions still
look stale.

## Remove Installed Files

If the project was installed with Autotools:

```sh
sudo make uninstall
```

For staged package roots, uninstall from the same configured build tree and
`DESTDIR`.

## Optional User and Group Removal

Only remove users or groups deliberately after confirming they are not used by
other local policy:

```sh
sudo userdel mochad
sudo groupdel mochad
sudo groupdel x10
```

Do not automate this step.

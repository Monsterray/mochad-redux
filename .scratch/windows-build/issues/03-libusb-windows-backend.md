# libusb on Windows: backend and driver for CM19A/CM15A

Type: research
Status: open
Blocked by: none

## Question

`libusb-1.0` is cross-platform, but on Windows it needs a kernel driver bound to
the device, and which one materially affects whether this project is usable at
all. Establish:

- Which backends current libusb supports on Windows (WinUSB, libusbK, libusb0)
  and which the CM19A (`0bc7:0002`) and CM15A (`0bc7:0001`) can use.
- Whether a user must run a tool such as Zadig to replace the driver, and what
  that displaces.
- Whether `libusb_detach_kernel_driver` and the interface-claim path the daemon
  uses work on Windows or return `NOT_SUPPORTED`.
- Whether hotplug (`libusb_hotplug_register_callback`, which the new
  `usb_recovery` state machine drives) is supported on the Windows backend.

The last point is load-bearing: if hotplug is unsupported, the recovery feature
just validated on macOS cannot work on Windows, and the map should say so rather
than discover it late.

Answer with citations to libusb documentation or source, not recollection.

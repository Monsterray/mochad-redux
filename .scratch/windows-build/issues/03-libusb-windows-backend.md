# libusb on Windows: backend and driver for CM19A/CM15A

Type: research
Status: resolved
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

## Answer

**Resolved: a manageable cost, not a blocker — and the code is already better
prepared than expected. The event loop, not USB access, is the real problem.**

### Backends and driver binding

libusb supports WinUSB (recommended default), libusbK, libusb-win32, and usbdk
on Windows; the native HID backend is "highly discouraged" by the project. All
of the first three are installed with Zadig. libusb's own docs: *"If your target
device is not HID, and your device is not using WinUSB driver, you must install
a driver before you can communicate with it using libusb."*
<https://github.com/libusb/libusb/wiki/Windows>

Two user-visible costs worth stating plainly:

- **WinUSB does not support multiple concurrent applications** opening the
  device. That is consistent with the project's existing guidance not to run two
  mochad processes against one controller, but it also means the daemon and X10's
  own Windows software cannot both hold the device.
- **Driver replacement is not cleanly reversible** to the native HID binding in
  all cases — reported against Zadig's own tooling in
  <https://github.com/pbatard/libwdi/issues/8> and
  <https://github.com/pbatard/libwdi/issues/120>.

### Hotplug: reversed, but immature

The historical answer was "unsupported on Windows". That changed: libusb
**1.0.30, released 2026-05-17**, lists *"Add hotplug support on Microsoft
Windows"*. Verified directly against the ChangeLog on `master`, not taken second
hand. <https://raw.githubusercontent.com/libusb/libusb/master/ChangeLog>

But it is **opt-in at build time** (`--enable-windows-hotplug`, defaulting to
`no`), shipped as a separate prebuilt variant, and still being actively
corrected — a follow-up PR was open as recently as 2026-08-13.
<https://github.com/libusb/libusb/pull/1930>

**The daemon already handles this correctly.** `mochad.c:1038` guards with
`if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))` and degrades to a logged
notice rather than failing. So a non-hotplug Windows build loses recovery but
does not break. No code change is required for correctness here; only the
support claim needs to be honest about it.

### Kernel-driver APIs: unsupported, but unreachable in the good case

`libusb_detach_kernel_driver` and `libusb_kernel_driver_active` both return
`LIBUSB_ERROR_NOT_SUPPORTED` on Windows — confirmed in `libusb/core.c` on
`master`, and consistent with the FAQ's "there is no Windows kernel driver
involved in the libusb project".

Reading the daemon's actual path (`mochad.c:1099-1132`), those calls sit in a
**fallback reached only when `libusb_claim_interface` fails**:

- `libusb_set_auto_detach_kernel_driver` failure is already tolerated (logged at
  `LOG_DEBUG`, execution continues).
- If `libusb_claim_interface` succeeds — which it should on Windows once WinUSB
  is bound — the function returns before touching either unsupported call.
- Only on claim failure does it reach `libusb_kernel_driver_active`, get a
  negative result, and `goto fail`.

So the practical Windows defect is **a misleading diagnostic, not a crash**: a
user who has not run Zadig gets "kernel driver check failed" and advice about
`ati_remote`, when the actionable message is "bind WinUSB to this device". Small,
worth fixing, graduated as ticket 13.

### Event loop — corroborates ticket 07 independently

`libusb/io.c` on `master`: `libusb_get_pollfds()` *"simply returns NULL"* on
Windows and applications *"are advised to use an event handling thread
instead."* This is unchanged by the 1.0.30 hotplug work and is architectural.
Same conclusion ticket 07 reached from the WSAPoll side, arrived at
independently.

### Composite/HID

The CM15A appears to be single-configuration, single-interface, so Windows'
composite parent driver is not implicated. Whether Windows binds the HID class
driver by default is **unconfirmed** at the device level; if it does, the remedy
is the same Zadig replacement already described, not a new class of problem.

### Impact

USB access is solved at the cost of a per-machine driver swap with real UX
consequences. Hotplug is newly possible but immature and non-default. Neither
blocks the destination. The event-loop rewrite identified in ticket 07 remains
the dominant cost.

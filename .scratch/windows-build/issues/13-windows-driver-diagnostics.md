# Windows driver-binding diagnostics and documentation

Type: task
Status: open
Blocked by: 02

## Question

Graduated from ticket 03. On Windows, `libusb_claim_interface` fails until a
libusb-compatible driver (normally WinUSB, via Zadig) is bound to the
controller. The daemon's failure path then calls `libusb_kernel_driver_active`,
which returns `LIBUSB_ERROR_NOT_SUPPORTED` on Windows, and reports:

    [USB] kernel driver check failed rc=... error=...
    [USB] kernel driver detach failed ...; check drivers such as ati_remote

That advice is Linux-specific and actively misleading on Windows, where the
actionable step is "bind WinUSB to this device". Not a crash — the good path
never reaches these calls once the driver is bound — but a bad first-run
experience on the platform where the extra setup step is mandatory.

Do:

- Give the claim-failure diagnostic a platform-appropriate branch, so Windows
  users are pointed at driver binding rather than `ati_remote`.
- Document the Zadig prerequisite wherever Windows install steps end up living,
  including the two costs ticket 03 established: WinUSB does not allow two
  applications to hold the device at once, and reverting to the original HID
  binding is not reliably clean.

Deliberately small and low-risk; it is grouped here rather than folded into a
larger ticket because it is the one Windows change that improves the experience
without touching the event loop.

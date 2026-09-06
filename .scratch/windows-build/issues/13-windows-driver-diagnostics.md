# Windows driver-binding diagnostics and documentation

Type: task
Status: partially done
Blocked by: 02 (Windows half only)

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

## Progress

The non-Windows half is done on `fix/sigpipe-suppression`, commit `679b847`,
because the same defect was live on macOS -- a platform the repository already
validates -- and did not need a Windows toolchain to fix.

Done:

- The advice is now in one `usb_claim_hint()` in `src/core/mochad.c`, branched
  per platform, so the two call sites that give it cannot drift apart.
- Linux keeps the `ati_remote` and udev guidance; macOS is told to look for
  another application holding the controller.
- `libusb_kernel_driver_active()` returning `LIBUSB_ERROR_NOT_SUPPORTED` is
  reported as the platform having no driver to detach, not as a check that
  failed. This was the more misleading of the two: it handed a macOS reader a
  driver fault to chase when the real cause was in the line above it.
- Verified by extracting the function and compiling each branch under
  `-Werror`; every branch yields distinct, non-empty advice.

Still blocked on 02:

- The `#elif defined(_WIN32)` branch pointing at WinUSB and Zadig. Adding it is
  trivial; adding it *verified* is not, and the map's standing constraint says
  unverified compilation is not to be reported as success. It is one line
  whenever there is a toolchain to compile it with.
- Documenting the Zadig prerequisite, including the two costs ticket 03
  established: WinUSB does not allow two applications to hold the device, and
  reverting to the original HID binding is not reliably clean. That has no
  natural home until 02 and 04 decide where Windows install steps live.

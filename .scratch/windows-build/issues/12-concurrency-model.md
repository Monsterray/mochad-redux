# Concurrency model once libusb moves to its own thread

Type: grilling
Status: open
Blocked by: none

## Question

Graduated from the fog by ticket 07, which established that Windows offers no
way to poll libusb alongside sockets: `WSAPoll` accepts only sockets, and
`libusb_get_pollfds()` returns NULL on Windows by design. The documented pattern
is a dedicated libusb event thread.

`src/core/mochad.c` is strictly single-threaded today — `grep -c
"pthread_mutex\|pthread_t\|_Atomic"` returns 0 — and the main loop interleaves
libusb descriptors with client and listener sockets in one `poll()`. Splitting
libusb onto its own thread therefore introduces the first shared mutable state
in the daemon.

Decide:

- **Ownership.** Which thread owns the client table, the per-client output
  queues, and X10 decode/dispatch state. A queue handing work to a single owner
  is usually cheaper to reason about than locks around shared structures.
- **Mechanism.** Mutex/critical section around shared structures, or a lock-free
  or locked queue between the USB thread and the socket thread.
- **Scope.** Whether this concurrency model becomes the design on *all*
  platforms, or is Windows-only behind `platform.h`. One model everywhere means
  more churn but a single code path to reason about and test; two models means
  POSIX keeps its proven single-threaded loop and Windows carries the risk.
- **Blast radius.** The audit found the daemon's buffer handling careful and its
  single-threaded assumptions load-bearing. Introducing threads reopens
  correctness questions that single-threading currently answers for free —
  including around the recently added `usb_recovery` state machine, which is
  driven from libusb hotplug callbacks and would now run on the USB thread.

This is the largest cost in the map and it was invisible when the effort was
scoped as "add a syslog shim". Settle it before any implementation ticket lands.

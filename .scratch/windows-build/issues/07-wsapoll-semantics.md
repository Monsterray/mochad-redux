# poll() to WSAPoll: semantic differences

Type: research
Status: resolved
Blocked by: none

## Question

`WSAPoll` is shaped like `poll` but is not a drop-in. Establish precisely:

- Whether it accepts a zero-length descriptor set, which the main loop can
  produce.
- Its documented `POLLOUT` defect and whether that still applies on current
  Windows.
- Whether it works only on sockets. This matters because the loop also polls the
  libusb-owned descriptor set.
- What replaces the POSIX behaviour for non-socket handles, if anything.

That last point may be decisive: if `WSAPoll` cannot poll libusb descriptors,
the main loop needs restructuring rather than aliasing, which is a far larger
change than this map currently assumes.

Answer with citations to Microsoft documentation and the libusb Windows backend
source.

## Answer

**Resolved: `poll` cannot be aliased to `WSAPoll`. This is a concurrency-model
redesign, not a syscall substitution.**

Two independent findings combine to rule out a unified event loop on Windows.

**WSAPoll is sockets-only.** No pipe, file, device handle or event object can be
placed in a `WSAPOLLFD`. Confirmed by the absence of any non-socket handle type
in the Microsoft reference, and corroborated by libuv, whose `uv_poll_t` docs
state the difference explicitly: "On windows only sockets can be polled with
poll handles. On Unix any file descriptor that would be accepted by poll(2) can
be used."
<https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsapoll>
<https://docs.libuv.org/en/v1.x/poll.html>

**libusb exposes nothing pollable on Windows.** The current libusb API docs say
`libusb_get_pollfds()` "simply returns NULL" on Windows and that "applications
which want to support Windows are advised to use an event handling thread
instead." The project wiki confirms this is architectural, not an oversight:
better native Windows event handling "will require a redesign of the libusb API,
so it probably won't occur before libusb 2.0." A 2020 effort to enable
`libusb_get_pollfds()` on Windows (issue #252) landed a platform-abstraction
layer in 1.0.24 but did not expose pollable handles through that API.
<https://libusb.sourceforge.io/api-1.0/group__libusb__poll.html>
<https://github.com/libusb/libusb/wiki/Windows>
<https://github.com/libusb/libusb/issues/252>

There is therefore no join point: libusb events cannot enter a `WSAPoll` array,
and sockets cannot enter libusb's world. The documented Windows pattern is a
dedicated thread calling `libusb_handle_events_completed()` in a loop.

**Verified locally against this codebase.** `src/core/mochad.c` collects libusb
descriptors into `UsbPollfds`/`NUsbPollfds` and merges them into the same poll
set as the client and listener sockets, so the POSIX design does rely on the
unified loop. And `grep -c "pthread_mutex\|pthread_t\|_Atomic" src/core/mochad.c`
returns **0**: the daemon is strictly single-threaded and shares client state
with no synchronization at all. Introducing a USB event thread therefore forces
an ownership and locking design around the client table and any USB-driven
writes into shared state.

**Socket-only portion is still tractable** as a shim rather than an alias:

- `nfds == 0` must be special-cased. The reference requires "at least one
  structure with a valid socket" and returns `WSAEINVAL` otherwise, where POSIX
  `poll` permits an empty set and simply waits out the timeout. The daemon can
  legitimately have nothing to poll.
- `POLLPRI` is explicitly unsupported and makes the call fail.
- A negative `fd` is not silently ignored as POSIX requires; Windows sets
  `POLLNVAL` in `revents`, so any `revents == 0` "skip" test misbehaves.
- Errors come from `WSAGetLastError()`, not `errno`.
- Timeout semantics are the one thing that genuinely maps 1:1.

**The famous WSAPoll defect does not apply here.** The failure to report a
failed non-blocking `connect()` — resolved "Won't Fix" by Microsoft in 2011, and
the reason curl removed WSAPoll in 2012 — concerns *outbound* connections. This
daemon only ever accepts. Partially fixed from Windows 10 2004 in any case.
<https://daniel.haxx.se/blog/2012/10/10/wsapoll-is-broken/>

**Consequence for the map:** graduates the concurrency question out of the fog
as ticket 12, and materially raises the cost of the destination. Ticket 05 must
now account for state shared across threads, not just handle-type differences.

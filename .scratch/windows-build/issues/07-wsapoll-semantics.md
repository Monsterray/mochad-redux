# poll() to WSAPoll: semantic differences

Type: research
Status: open
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

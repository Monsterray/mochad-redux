# Shape of the socket abstraction

Type: grilling
Status: open
Blocked by: 02

## Question

Windows separates socket handles from file descriptors: sockets are `SOCKET`
(an unsigned handle whose invalid value is `INVALID_SOCKET`, not `-1`), closed
with `closesocket`, and read and written with `recv`/`send` rather than
`read`/`write`. The daemon stores descriptors as `int` and mixes socket and
non-socket `close()` calls — 12 `close()` sites, only some of them sockets.

Decide the shape: a `mochad_socket_t` typedef plus macros, thin wrapper
functions, or keeping `int` and relying on MinGW compatibility shims. The choice
determines how invasive the change is and how the 12 `close()` sites get
disambiguated.

The error convention differs too: Winsock reports through `WSAGetLastError()`,
not `errno`, so any `errno` check on a socket path needs attention.

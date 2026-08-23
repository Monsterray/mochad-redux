# Non-blocking sockets and FIONREAD on Windows

Type: task
Status: open
Blocked by: 02, 05

## Question

Replace `fcntl(fd, F_SETFL, O_NONBLOCK)` (2 sites) and `ioctl(fd, FIONREAD)`
(1 site) with `ioctlsocket(FIONBIO)` and `ioctlsocket(FIONREAD)` behind
`platform.h`. Confirm first whether any of those calls target a non-socket
descriptor, because a socket-only replacement would then be wrong.

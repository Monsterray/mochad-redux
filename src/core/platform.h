/*
 * platform.h - the single place where operating-system differences live.
 *
 * Copyright 2010-2011 Brian Uechi <buasst@gmail.com>
 *
 * This file is part of mochad.
 *
 * mochad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mochad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mochad.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The daemon is POSIX software. Rather than scatter platform headers and
 * feature probes across the tree, every OS-dependent include and fallback is
 * collected here, so the portability surface is one auditable file instead of
 * seven.
 *
 * The X10 protocol, USB, encode/decode and diagnostics code is already
 * portable C11 and needs nothing from this header. What is not portable is the
 * surrounding service layer:
 *
 *   syslog.h    logging               Windows has no syslog; it has its own
 *                                     event log with a different model.
 *   unistd.h    close/read/write      Windows separates socket handles from
 *   poll.h      readiness polling     file descriptors and spells these
 *   sys/socket.h socket API           closesocket/recv/send/WSAPoll.
 *   sys/ioctl.h  FIONREAD             Windows uses ioctlsocket.
 *   netinet/in.h address types        Winsock supplies equivalents in
 *   arpa/inet.h                       winsock2.h / ws2tcpip.h.
 *   netdb.h      getaddrinfo
 *   signal.h     SIGINT/SIGTERM       Windows uses console control handlers.
 *   fcntl.h      O_NONBLOCK           Winsock uses ioctlsocket(FIONBIO).
 *
 * Adding Windows support means implementing those behind this header. See
 * docs/development/portability.md for the current assessment. Nothing here
 * changes behaviour on POSIX: the includes below are exactly the ones the
 * sources used before, in the same order.
 */

#ifndef MOCHAD_PLATFORM_H
#define MOCHAD_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
#error "mochad-redux does not build on Windows yet. The POSIX service layer \
(syslog, poll, daemon, BSD sockets) has no implementation here. See \
docs/development/portability.md. Build under WSL2 or a Linux/macOS host."
#endif

/* POSIX: Linux, macOS, and the BSDs. */
#include <signal.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * Not defined on every POSIX platform -- notably absent on macOS and the
 * BSDs, which offer SO_NOSIGPIPE per socket instead. Falling back to 0 keeps
 * send() calls valid but silently removes the protection on its own,
 * so mochad_ignore_sigpipe() (src/net/socket_io.c), called once from
 * mydaemon() before any listener exists, carries it on those platforms.
 */
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#endif /* MOCHAD_PLATFORM_H */

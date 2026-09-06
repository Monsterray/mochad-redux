/*
 * Socket output helpers.
 */

#ifndef MOCHAD_SOCKET_IO_H
#define MOCHAD_SOCKET_IO_H

#include <stddef.h>
#include <sys/types.h>

typedef ssize_t (*mochad_send_func)(int fd, const void *buffer, size_t length, int flags,
                                    void *context);

int send_all(int fd, const void *buffer, size_t length);

/*
 * Ignore SIGPIPE for the whole process.
 *
 * send() is asked for MSG_NOSIGNAL, but that flag does not exist on macOS or
 * the BSDs; there the fallback below compiles it to 0 and a peer that closed
 * its end kills the process instead of failing the call. Ignoring the signal
 * makes send() and write() return -1 with EPIPE on every platform, which is
 * what the callers here already expect.
 *
 * Returns 0 on success, -1 with errno set.
 */
int mochad_ignore_sigpipe(void);

#ifdef MOCHAD_TESTING
int send_all_with_sender(int fd, const void *buffer, size_t length, mochad_send_func sender,
                         void *context);
#endif

#endif

/*
 * Socket output helpers.
 */

#ifndef MOCHAD_SOCKET_IO_H
#define MOCHAD_SOCKET_IO_H

#include <stddef.h>
#include <sys/types.h>

typedef ssize_t (*mochad_send_func)(int fd, const void *buffer,
        size_t length, int flags, void *context);

int send_all(int fd, const void *buffer, size_t length);

#ifdef MOCHAD_TESTING
int send_all_with_sender(int fd, const void *buffer, size_t length,
        mochad_send_func sender, void *context);
#endif

#endif

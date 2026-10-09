/*
 * Socket output helpers.
 */

#include "socket_io.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <syslog.h>

#include "platform.h"

int mochad_client_disconnect_log_level(const mochad_client_activity *activity, uint64_t now_ms) {
    /* Short, command-free connections are normally TCP health probes. */
    if (activity->received_bytes == 0 && now_ms >= activity->connected_ms &&
        now_ms - activity->connected_ms < 5000)
        return LOG_DEBUG;
    return LOG_NOTICE;
}

int mochad_ignore_sigpipe(void) {
    struct sigaction ignore;

    memset(&ignore, 0, sizeof(ignore));
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    ignore.sa_flags = 0;

    return sigaction(SIGPIPE, &ignore, NULL);
}

static ssize_t real_send(int fd, const void *buffer, size_t length, int flags, void *context) {
    (void)context;
    return send(fd, buffer, length, flags);
}

static int send_all_impl(int fd, const void *buffer, size_t length, mochad_send_func sender,
                         void *context) {
    const unsigned char *cursor = buffer;
    size_t remaining = length;

    if (length > (size_t)INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }

    while (remaining > 0) {
        ssize_t written = sender(fd, cursor, remaining, MSG_NOSIGNAL, context);

        if (written > 0) {
            cursor += written;
            remaining -= (size_t)written;
            continue;
        }

        if (written == 0) {
            errno = EPIPE;
            return -1;
        }

        if (errno == EINTR)
            continue;

        return -1;
    }

    return (int)length;
}

int send_all(int fd, const void *buffer, size_t length) {
    return send_all_impl(fd, buffer, length, real_send, NULL);
}

#ifdef MOCHAD_TESTING
int send_all_with_sender(int fd, const void *buffer, size_t length, mochad_send_func sender,
                         void *context) {
    if (sender == NULL) {
        errno = EINVAL;
        return -1;
    }

    return send_all_impl(fd, buffer, length, sender, context);
}
#endif

#include "socket_io.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct fake_sender_state {
    char bytes[128];
    size_t used;
    size_t max_chunk;
    int interrupt_once;
    int return_zero;
};

static ssize_t fake_sender(int fd, const void *buffer, size_t length, int flags, void *context) {
    struct fake_sender_state *state = context;
    size_t chunk;

    (void)fd;
    (void)flags;

    if (state->interrupt_once) {
        state->interrupt_once = 0;
        errno = EINTR;
        return -1;
    }

    if (state->return_zero)
        return 0;

    chunk = length < state->max_chunk ? length : state->max_chunk;
    if (chunk > sizeof(state->bytes) - state->used)
        chunk = sizeof(state->bytes) - state->used;

    memcpy(state->bytes + state->used, buffer, chunk);
    state->used += chunk;
    return (ssize_t)chunk;
}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int test_partial_writes(void) {
    const char payload[] = "partial-write-safe";
    struct fake_sender_state state;

    memset(&state, 0, sizeof(state));
    state.max_chunk = 3;

    if (send_all_with_sender(7, payload, strlen(payload), fake_sender, &state) !=
        (int)strlen(payload))
        return expect(0, "send_all_with_sender returned wrong length");

    return expect(state.used == strlen(payload) &&
                      memcmp(state.bytes, payload, strlen(payload)) == 0,
                  "partial writes did not preserve payload");
}

static int test_eintr_retry(void) {
    const char payload[] = "retry";
    struct fake_sender_state state;

    memset(&state, 0, sizeof(state));
    state.max_chunk = 2;
    state.interrupt_once = 1;

    if (send_all_with_sender(7, payload, strlen(payload), fake_sender, &state) !=
        (int)strlen(payload))
        return expect(0, "send_all_with_sender did not retry EINTR");

    return expect(state.used == strlen(payload) &&
                      memcmp(state.bytes, payload, strlen(payload)) == 0,
                  "EINTR retry payload mismatch");
}

static int test_zero_write_fails(void) {
    const char payload[] = "closed";
    struct fake_sender_state state;

    memset(&state, 0, sizeof(state));
    state.max_chunk = 2;
    state.return_zero = 1;

    if (send_all_with_sender(7, payload, strlen(payload), fake_sender, &state) != -1)
        return expect(0, "zero-length send should fail");

    return expect(errno == EPIPE, "zero-length send should set EPIPE");
}

static int test_real_socketpair(void) {
    const char payload[] = "socketpair";
    char received[32];
    int sockets[2];
    ssize_t bytes;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
        perror("socketpair");
        return 1;
    }

    if (send_all(sockets[0], payload, strlen(payload)) != (int)strlen(payload)) {
        perror("send_all");
        close(sockets[0]);
        close(sockets[1]);
        return 1;
    }

    bytes = read(sockets[1], received, sizeof(received));
    close(sockets[0]);
    close(sockets[1]);

    if (bytes != (ssize_t)strlen(payload))
        return expect(0, "socketpair read length mismatch");

    return expect(memcmp(received, payload, strlen(payload)) == 0, "socketpair payload mismatch");
}

int main(void) {
    if (test_partial_writes())
        return 1;
    if (test_eintr_retry())
        return 1;
    if (test_zero_write_fails())
        return 1;
    if (test_real_socketpair())
        return 1;

    puts("PASS: socket_io");
    return 0;
}

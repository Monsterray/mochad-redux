/*
 * SIGPIPE suppression on platforms without MSG_NOSIGNAL.
 *
 * MSG_NOSIGNAL is a Linux extension.  macOS and the BSDs do not have it, so
 * src/net/socket_io.c falls it back to 0 -- which compiles, but silently
 * removes the protection: the default disposition of SIGPIPE terminates the
 * process, so one unauthenticated client disconnecting between a queued
 * response and the next flush takes the daemon and every other client with it.
 *
 * These tests emulate such a platform by passing flags = 0 to send(), which is
 * exactly what the fallback produces, and check both halves of the claim:
 *
 *   1. without suppression the process really is killed  (the bug is real)
 *   2. with mochad_ignore_sigpipe() it survives with EPIPE  (the fix works)
 *
 * Each case runs in a forked child because case 1 is expected to die, and
 * because a signal disposition is process-wide: the parent must never call
 * mochad_ignore_sigpipe() itself, or case 1 would inherit the ignore and pass
 * for the wrong reason.
 */

#include "socket_io.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

/* Child exit codes. */
#define CHILD_EPIPE 0 /* write failed with EPIPE, as it should */
#define CHILD_UNEXPECTED_OK 1
#define CHILD_WRONG_ERRNO 2
#define CHILD_SETUP_FAILED 3

static int failures;

static void check(int condition, const char *what) {
    if (condition) {
        printf("  ok: %s\n", what);
        return;
    }
    printf("  FAIL: %s\n", what);
    failures++;
}

/*
 * Open a stream socket pair and close the peer, so the next write to the
 * returned descriptor has nowhere to go.  The pair is created here, inside the
 * child, so no descriptor for the peer survives anywhere else in the process
 * tree -- a stray open copy in the parent would keep the pipe alive and the
 * test would silently prove nothing.
 */
static int broken_socket(void) {
    int pair[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0)
        return -1;
    if (close(pair[1]) != 0) {
        close(pair[0]);
        return -1;
    }
    return pair[0];
}

static int classify(ssize_t written) {
    if (written >= 0)
        return CHILD_UNEXPECTED_OK;
    if (errno != EPIPE)
        return CHILD_WRONG_ERRNO;
    return CHILD_EPIPE;
}

/* Emulates a host whose send() has no MSG_NOSIGNAL to offer. */
static ssize_t send_flagless(int fd, const void *buffer, size_t length, int flags, void *context) {
    (void)flags;
    (void)context;
    return send(fd, buffer, length, 0);
}

static int child_raw_send(int suppress) {
    int fd = broken_socket();

    if (fd < 0)
        return CHILD_SETUP_FAILED;
    if (suppress && mochad_ignore_sigpipe() != 0)
        return CHILD_SETUP_FAILED;

    return classify(send(fd, "x", 1, 0));
}

static int child_send_all(int unused) {
    int fd = broken_socket();

    (void)unused;
    if (fd < 0)
        return CHILD_SETUP_FAILED;
    if (mochad_ignore_sigpipe() != 0)
        return CHILD_SETUP_FAILED;

    return classify(send_all_with_sender(fd, "x", 1, send_flagless, NULL));
}

/* Runs body in a child and reports the raw wait status. */
static int run_child(int (*body)(int), int argument, int *status) {
    pid_t pid;

    fflush(stdout);
    pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0)
        _exit(body(argument));

    return waitpid(pid, status, 0) == pid ? 0 : -1;
}

int main(void) {
    int status;

    printf("== SIGPIPE suppression ==\n");

    printf("without suppression, a write to a closed peer kills the process\n");
    if (run_child(child_raw_send, 0, &status) != 0) {
        printf("  FAIL: could not run child\n");
        return 1;
    }
    check(WIFSIGNALED(status) && WTERMSIG(status) == SIGPIPE,
          "child terminated by SIGPIPE (the defect this guards against)");

    printf("with mochad_ignore_sigpipe(), the same write fails with EPIPE\n");
    if (run_child(child_raw_send, 1, &status) != 0) {
        printf("  FAIL: could not run child\n");
        return 1;
    }
    check(!WIFSIGNALED(status), "child was not killed by a signal");
    check(WIFEXITED(status) && WEXITSTATUS(status) == CHILD_EPIPE, "send() returned -1 with EPIPE");

    printf("send_all() survives the same disconnect on a flagless host\n");
    if (run_child(child_send_all, 0, &status) != 0) {
        printf("  FAIL: could not run child\n");
        return 1;
    }
    check(!WIFSIGNALED(status), "child was not killed by a signal");
    check(WIFEXITED(status) && WEXITSTATUS(status) == CHILD_EPIPE,
          "send_all() reported failure instead of dying");

    if (failures) {
        printf("FAIL: %d check(s)\n", failures);
        return 1;
    }
    printf("PASS: sigpipe\n");
    return 0;
}

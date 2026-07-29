#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "global.h"
#include "transport_evidence.h"
#include "x10_write.h"

int PollTimeOut = -1;

static int write_usb_result = 0;
static int write_usb_calls = 0;
static unsigned char last_write[8];
static size_t last_write_len = 0;

int _dbprintf(const char *fmt, ...) {
    (void)fmt;
    return 0;
}

int write_usb(unsigned char *buf, size_t len) {
    write_usb_calls++;
    last_write_len = len;
    memcpy(last_write, buf, len);
    return write_usb_result;
}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int test_initial_submit_failure_does_not_leave_queue_busy(void) {
    unsigned char first[] = {0x01, 0x02};
    unsigned char second[] = {0x03, 0x04};

    write_usb_result = -EIO;
    if (expect(x10_write(first, sizeof(first)) == -EIO, "initial write failure should propagate"))
        return 1;
    if (expect(PollTimeOut == -1, "initial write failure should leave polling idle"))
        return 1;

    write_usb_result = 0;
    if (expect(x10_write(second, sizeof(second)) == (int)sizeof(second),
               "second write should submit after first failure"))
        return 1;
    if (expect(last_write_len == sizeof(second) && memcmp(last_write, second, sizeof(second)) == 0,
               "second write did not reach USB after first failure"))
        return 1;

    return expect(PollTimeOut == 2000, "successful write should arm queue timeout");
}

static int test_queued_submit_failure_does_not_advance_head(void) {
    unsigned char queued[] = {0x05, 0x06, 0x07};
    int calls_before;

    if (expect(x10_write(queued, sizeof(queued)) == (int)sizeof(queued), "busy write should queue"))
        return 1;

    calls_before = write_usb_calls;
    write_usb_result = -EBUSY;
    if (expect(send_next_x10out() == -EBUSY, "queued submit failure should propagate"))
        return 1;
    if (expect(write_usb_calls == calls_before + 1,
               "queued submit failure should attempt exactly one USB write"))
        return 1;

    write_usb_result = 0;
    if (expect(send_next_x10out() == 0, "queued item should retry after submit failure"))
        return 1;
    return expect(last_write_len == sizeof(queued) &&
                      memcmp(last_write, queued, sizeof(queued)) == 0,
                  "queued item changed after failed submit retry");
}

static int test_shutdown_cancels_pending_attempts(void) {
    char evidence[16384];
    unsigned char queued[] = {0x08, 0x09};

    if (expect(x10_write(queued, sizeof(queued)) == (int)sizeof(queued),
               "busy write should queue before shutdown"))
        return 1;

    cancel_pending_x10out();
    if (expect(mochad_transport_evidence_json(evidence, sizeof(evidence)) > 0,
               "shutdown evidence should fit"))
        return 1;
    return expect(strstr(evidence, "\"reason\":\"shutdown_before_submission\"") != NULL,
                  "shutdown should terminate queued evidence");
}

int main(void) {
    mochad_transport_evidence_reset();
    if (test_initial_submit_failure_does_not_leave_queue_busy())
        return 1;
    if (test_queued_submit_failure_does_not_advance_head())
        return 1;
    if (test_shutdown_cancels_pending_attempts())
        return 1;

    puts("PASS: x10_write");
    return 0;
}

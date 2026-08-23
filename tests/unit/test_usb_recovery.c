#include "usb_recovery.h"

#include <stdio.h>

#define CM19A_PRODUCT_ID 0x0002
#define CM15A_PRODUCT_ID 0x0001

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int test_disconnect_and_recovery(void) {
    mochad_usb_recovery recovery;

    mochad_usb_recovery_init(&recovery, CM19A_PRODUCT_ID, 1);
    mochad_usb_recovery_removed(&recovery, CM15A_PRODUCT_ID);
    if (expect(mochad_usb_recovery_ready(&recovery), "other controller removal changed state"))
        return 1;

    mochad_usb_recovery_removed(&recovery, CM19A_PRODUCT_ID);
    if (expect(mochad_usb_recovery_timeout_ms(&recovery, 1000) == 100,
               "detach did not bound event-loop polling"))
        return 1;
    if (expect(mochad_usb_recovery_next(&recovery, 1000) == MOCHAD_USB_ACTION_DETACH,
               "active controller removal did not request detach"))
        return 1;
    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    mochad_usb_recovery_detached(&recovery);
    if (expect(mochad_usb_recovery_next(&recovery, 1000) == MOCHAD_USB_ACTION_ATTACH,
               "arrival during detach did not request attach"))
        return 1;
    mochad_usb_recovery_attach_result(&recovery, 1, 1000);
    return expect(mochad_usb_recovery_ready(&recovery), "successful attach did not become ready");
}

static int test_retry_and_duplicates(void) {
    mochad_usb_recovery recovery;

    mochad_usb_recovery_init(&recovery, CM19A_PRODUCT_ID, 1);
    mochad_usb_recovery_removed(&recovery, CM19A_PRODUCT_ID);
    mochad_usb_recovery_removed(&recovery, CM19A_PRODUCT_ID);
    mochad_usb_recovery_detached(&recovery);
    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    if (expect(mochad_usb_recovery_next(&recovery, 2000) == MOCHAD_USB_ACTION_ATTACH,
               "arrival did not start recovery"))
        return 1;

    mochad_usb_recovery_attach_result(&recovery, 0, 2000);
    if (expect(mochad_usb_recovery_timeout_ms(&recovery, 2200) == 300,
               "failed recovery did not schedule bounded retry") ||
        expect(mochad_usb_recovery_next(&recovery, 2499) == MOCHAD_USB_ACTION_NONE,
               "recovery retried too early") ||
        expect(mochad_usb_recovery_next(&recovery, 2500) == MOCHAD_USB_ACTION_ATTACH,
               "recovery did not retry at deadline"))
        return 1;

    mochad_usb_recovery_attach_result(&recovery, 1, 2500);
    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    return expect(mochad_usb_recovery_ready(&recovery), "duplicate arrival changed ready state");
}

static int test_disconnect_without_arrival_waits(void) {
    mochad_usb_recovery recovery;

    mochad_usb_recovery_init(&recovery, CM19A_PRODUCT_ID, 1);
    mochad_usb_recovery_removed(&recovery, CM19A_PRODUCT_ID);
    mochad_usb_recovery_detached(&recovery);
    return expect(mochad_usb_recovery_next(&recovery, 5000) == MOCHAD_USB_ACTION_NONE,
                  "disconnected controller retried before arrival");
}

static int test_retry_limit_waits_for_new_arrival(void) {
    mochad_usb_recovery recovery;
    unsigned int attempt;

    mochad_usb_recovery_init(&recovery, CM19A_PRODUCT_ID, 0);
    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    for (attempt = 0; attempt < 10; attempt++) {
        if (expect(mochad_usb_recovery_next(&recovery, attempt * 500U) == MOCHAD_USB_ACTION_ATTACH,
                   "bounded retry did not request attach"))
            return 1;
        mochad_usb_recovery_attach_result(&recovery, 0, attempt * 500U);
    }

    if (expect(!mochad_usb_recovery_retrying(&recovery), "retry limit did not stop retries") ||
        expect(mochad_usb_recovery_next(&recovery, 10000) == MOCHAD_USB_ACTION_NONE,
               "retry limit did not wait for a new arrival"))
        return 1;

    mochad_usb_recovery_arrived(&recovery, CM19A_PRODUCT_ID);
    return expect(mochad_usb_recovery_next(&recovery, 10000) == MOCHAD_USB_ACTION_ATTACH,
                  "new arrival did not reset retry limit");
}

int main(void) {
    if (test_disconnect_and_recovery() || test_retry_and_duplicates() ||
        test_disconnect_without_arrival_waits() || test_retry_limit_waits_for_new_arrival())
        return 1;

    puts("PASS: usb_recovery");
    return 0;
}

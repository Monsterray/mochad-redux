#include "usb_recovery.h"

#include <limits.h>

#define RECOVERY_RETRY_MS 500U
#define RECOVERY_RETRY_LIMIT 10U
#define DETACH_POLL_MS 100

void mochad_usb_recovery_init(mochad_usb_recovery *recovery, uint16_t product, int ready) {
    recovery->state = ready ? MOCHAD_USB_READY : MOCHAD_USB_DISCONNECTED;
    recovery->expected_product = product;
    recovery->retry_at_ms = 0;
    recovery->retry_count = 0;
    recovery->arrival_seen = ready;
}

void mochad_usb_recovery_removed(mochad_usb_recovery *recovery, uint16_t product) {
    if (product != recovery->expected_product)
        return;

    if (recovery->state == MOCHAD_USB_READY || recovery->state == MOCHAD_USB_RECOVERING) {
        recovery->state = MOCHAD_USB_DETACHING;
        recovery->arrival_seen = 0;
        recovery->retry_at_ms = 0;
    }
}

void mochad_usb_recovery_arrived(mochad_usb_recovery *recovery, uint16_t product) {
    if (product != recovery->expected_product)
        return;

    recovery->arrival_seen = 1;
    if (recovery->state == MOCHAD_USB_DISCONNECTED || recovery->state == MOCHAD_USB_WAITING) {
        recovery->retry_count = 0;
        recovery->state = MOCHAD_USB_ATTACH_PENDING;
    }
}

mochad_usb_recovery_action mochad_usb_recovery_next(mochad_usb_recovery *recovery,
                                                    uint64_t now_ms) {
    if (recovery->state == MOCHAD_USB_DETACHING)
        return MOCHAD_USB_ACTION_DETACH;

    if (recovery->state == MOCHAD_USB_ATTACH_PENDING ||
        (recovery->state == MOCHAD_USB_WAITING && recovery->arrival_seen &&
         now_ms >= recovery->retry_at_ms)) {
        recovery->state = MOCHAD_USB_RECOVERING;
        return MOCHAD_USB_ACTION_ATTACH;
    }

    return MOCHAD_USB_ACTION_NONE;
}

void mochad_usb_recovery_detached(mochad_usb_recovery *recovery) {
    recovery->state = recovery->arrival_seen ? MOCHAD_USB_ATTACH_PENDING : MOCHAD_USB_DISCONNECTED;
}

void mochad_usb_recovery_attach_result(mochad_usb_recovery *recovery, int succeeded,
                                       uint64_t now_ms) {
    if (succeeded) {
        recovery->state = MOCHAD_USB_READY;
        recovery->arrival_seen = 1;
        recovery->retry_at_ms = 0;
        recovery->retry_count = 0;
        return;
    }

    recovery->retry_count++;
    if (recovery->retry_count >= RECOVERY_RETRY_LIMIT) {
        recovery->state = MOCHAD_USB_DISCONNECTED;
        recovery->arrival_seen = 0;
        recovery->retry_at_ms = 0;
        return;
    }
    recovery->state = MOCHAD_USB_WAITING;
    recovery->retry_at_ms = now_ms + RECOVERY_RETRY_MS;
}

int mochad_usb_recovery_timeout_ms(const mochad_usb_recovery *recovery, uint64_t now_ms) {
    uint64_t remaining;

    if (recovery->state == MOCHAD_USB_DETACHING)
        return DETACH_POLL_MS;
    if (recovery->state != MOCHAD_USB_WAITING || !recovery->arrival_seen)
        return -1;
    if (now_ms >= recovery->retry_at_ms)
        return 0;

    remaining = recovery->retry_at_ms - now_ms;
    return remaining > INT_MAX ? INT_MAX : (int)remaining;
}

int mochad_usb_recovery_ready(const mochad_usb_recovery *recovery) {
    return recovery->state == MOCHAD_USB_READY;
}

int mochad_usb_recovery_retrying(const mochad_usb_recovery *recovery) {
    return recovery->state == MOCHAD_USB_WAITING;
}

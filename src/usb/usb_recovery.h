#ifndef MOCHAD_USB_RECOVERY_H
#define MOCHAD_USB_RECOVERY_H

#include <stdint.h>

typedef enum {
    MOCHAD_USB_DISCONNECTED,
    MOCHAD_USB_DETACHING,
    MOCHAD_USB_WAITING,
    MOCHAD_USB_ATTACH_PENDING,
    MOCHAD_USB_RECOVERING,
    MOCHAD_USB_READY
} mochad_usb_recovery_state;

typedef enum {
    MOCHAD_USB_ACTION_NONE,
    MOCHAD_USB_ACTION_DETACH,
    MOCHAD_USB_ACTION_ATTACH
} mochad_usb_recovery_action;

typedef struct {
    mochad_usb_recovery_state state;
    uint16_t expected_product;
    uint64_t retry_at_ms;
    unsigned int retry_count;
    int arrival_seen;
} mochad_usb_recovery;

void mochad_usb_recovery_init(mochad_usb_recovery *recovery, uint16_t product, int ready);
void mochad_usb_recovery_removed(mochad_usb_recovery *recovery, uint16_t product);
void mochad_usb_recovery_arrived(mochad_usb_recovery *recovery, uint16_t product);
mochad_usb_recovery_action mochad_usb_recovery_next(mochad_usb_recovery *recovery, uint64_t now_ms);
void mochad_usb_recovery_detached(mochad_usb_recovery *recovery);
void mochad_usb_recovery_attach_result(mochad_usb_recovery *recovery, int succeeded,
                                       uint64_t now_ms);
int mochad_usb_recovery_timeout_ms(const mochad_usb_recovery *recovery, uint64_t now_ms);
int mochad_usb_recovery_ready(const mochad_usb_recovery *recovery);
int mochad_usb_recovery_retrying(const mochad_usb_recovery *recovery);

#endif

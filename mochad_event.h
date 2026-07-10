#ifndef MOCHAD_EVENT_H
#define MOCHAD_EVENT_H

#include <stddef.h>

typedef enum {
    MOCHAD_EVENT_DIRECTION_RX,
    MOCHAD_EVENT_DIRECTION_TX
} mochad_event_direction_t;

typedef enum {
    MOCHAD_EVENT_TRANSPORT_PL,
    MOCHAD_EVENT_TRANSPORT_RF,
    MOCHAD_EVENT_TRANSPORT_RFSEC,
    MOCHAD_EVENT_TRANSPORT_RFCAM
} mochad_event_transport_t;

typedef enum {
    MOCHAD_EVENT_DETAIL_HOUSE_UNIT,
    MOCHAD_EVENT_DETAIL_HOUSE,
    MOCHAD_EVENT_DETAIL_RFSEC_SHORT,
    MOCHAD_EVENT_DETAIL_RFSEC_LONG,
    MOCHAD_EVENT_DETAIL_RFCAM
} mochad_event_detail_t;

typedef struct {
    mochad_event_direction_t direction;
    mochad_event_transport_t transport;
    mochad_event_detail_t detail;
    char house;
    int unit;
    const char *function;
    int has_level;
    int level;
    int has_extended_data;
    unsigned int extended_data;
    unsigned int extended_command;
    unsigned int security_short_addr;
    unsigned char security_long_addr[3];
    const char *camera_command;
} mochad_event_t;

int mochad_event_format_legacy_body(const mochad_event_t *event,
        char *buffer, size_t buffer_len);
int mochad_event_format_legacy_line(const mochad_event_t *event,
        char *buffer, size_t buffer_len);
int mochad_event_format_xmlsocket_line(const mochad_event_t *event,
        char *buffer, size_t buffer_len);
int mochad_event_format_openremote_line(const mochad_event_t *event,
        char *buffer, size_t buffer_len);
int mochad_dispatch_event(int fd, const mochad_event_t *event);

#endif

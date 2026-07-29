#include "mochad_event.h"

#include "global.h"
#include "transport_evidence.h"

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static const char *direction_label(mochad_event_direction_t direction) {
    return direction == MOCHAD_EVENT_DIRECTION_TX ? "Tx" : "Rx";
}

static const char *transport_label(mochad_event_transport_t transport) {
    switch (transport) {
    case MOCHAD_EVENT_TRANSPORT_PL:
        return "PL";
    case MOCHAD_EVENT_TRANSPORT_RF:
        return "RF";
    case MOCHAD_EVENT_TRANSPORT_RFSEC:
        return "RFSEC";
    case MOCHAD_EVENT_TRANSPORT_RFCAM:
        return "RFCAM";
    }

    return "UNKNOWN";
}

static const char *evidence_direction(mochad_event_direction_t direction) {
    return direction == MOCHAD_EVENT_DIRECTION_TX ? "tx" : "rx";
}

static const char *evidence_transport(mochad_event_transport_t transport) {
    switch (transport) {
    case MOCHAD_EVENT_TRANSPORT_PL:
        return "powerline";
    case MOCHAD_EVENT_TRANSPORT_RF:
        return "rf";
    case MOCHAD_EVENT_TRANSPORT_RFSEC:
        return "security_rf";
    case MOCHAD_EVENT_TRANSPORT_RFCAM:
        return "camera_rf";
    }

    return "unknown";
}

static int format_checked(char *buffer, size_t buffer_len, const char *fmt, ...) {
    va_list args;
    int written;

    if (buffer_len == 0) {
        errno = EINVAL;
        return -1;
    }

    va_start(args, fmt);
    written = vsnprintf(buffer, buffer_len, fmt, args);
    va_end(args);

    if (written < 0)
        return -1;

    if ((size_t)written >= buffer_len) {
        errno = ENOSPC;
        return -1;
    }

    return written;
}

int mochad_event_format_legacy_body(const mochad_event_t *event, char *buffer, size_t buffer_len) {
    const char *direction;
    const char *transport;

    if (event == NULL || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    direction = direction_label(event->direction);
    transport = transport_label(event->transport);

    switch (event->detail) {
    case MOCHAD_EVENT_DETAIL_HOUSE_UNIT:
        if (event->has_extended_data) {
            return format_checked(buffer, buffer_len,
                                  "%s %s HouseUnit: %c%d Func: %s Data: %02X Command: %02X",
                                  direction, transport, event->house, event->unit, event->function,
                                  event->extended_data, event->extended_command);
        }
        if (event->function == NULL) {
            return format_checked(buffer, buffer_len, "%s %s HouseUnit: %c%d", direction, transport,
                                  event->house, event->unit);
        }
        return format_checked(buffer, buffer_len, "%s %s HouseUnit: %c%d Func: %s", direction,
                              transport, event->house, event->unit, event->function);

    case MOCHAD_EVENT_DETAIL_HOUSE:
        if (event->has_level) {
            return format_checked(buffer, buffer_len, "%s %s House: %c Func: %s(%d)", direction,
                                  transport, event->house, event->function, event->level);
        }
        return format_checked(buffer, buffer_len, "%s %s House: %c Func: %s", direction, transport,
                              event->house, event->function);

    case MOCHAD_EVENT_DETAIL_RFSEC_SHORT:
        return format_checked(buffer, buffer_len, "%s %s Addr: 0x%02X Func: %s", direction,
                              transport, event->security_short_addr, event->function);

    case MOCHAD_EVENT_DETAIL_RFSEC_LONG:
        return format_checked(buffer, buffer_len, "%s %s Addr: %02X:%02X:%02X Func: %s", direction,
                              transport, event->security_long_addr[0], event->security_long_addr[1],
                              event->security_long_addr[2], event->function);

    case MOCHAD_EVENT_DETAIL_RFCAM:
        return format_checked(buffer, buffer_len, "%s %s %s", direction, transport,
                              event->camera_command);
    }

    errno = EINVAL;
    return -1;
}

int mochad_event_format_legacy_line(const mochad_event_t *event, char *buffer, size_t buffer_len) {
    int written;

    written = mochad_event_format_legacy_body(event, buffer, buffer_len);
    if (written < 0)
        return -1;

    if ((size_t)written + 1 >= buffer_len) {
        errno = ENOSPC;
        return -1;
    }

    buffer[written] = '\n';
    buffer[written + 1] = '\0';
    return written + 1;
}

int mochad_event_format_xmlsocket_line(const mochad_event_t *event, char *buffer,
                                       size_t buffer_len) {
    int written;

    written = mochad_event_format_legacy_body(event, buffer, buffer_len);
    if (written < 0)
        return -1;

    if ((size_t)written + 1 >= buffer_len) {
        errno = ENOSPC;
        return -1;
    }

    buffer[written] = '\0';
    buffer[written + 1] = '\0';
    return written + 1;
}

int mochad_event_format_openremote_line(const mochad_event_t *event, char *buffer,
                                        size_t buffer_len) {
    return mochad_event_format_legacy_line(event, buffer, buffer_len);
}

int mochad_dispatch_event(int fd, const mochad_event_t *event) {
    char body[256];

    if (mochad_event_format_legacy_body(event, body, sizeof(body)) < 0)
        return -1;

    mochad_transport_evidence_receive(evidence_direction(event->direction),
                                      evidence_transport(event->transport), "decoded");
    return sockprintf(fd, "%s\n", body);
}

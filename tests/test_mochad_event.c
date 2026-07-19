#include "mochad_event.h"

#include <stdio.h>
#include <string.h>

#include "global.h"

int sockprintf(int fd, const char *fmt, ...) {
    (void)fd;
    (void)fmt;
    return 0;
}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int expect_text(const char *actual, const char *expected, const char *message) {
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: %s\nexpected: %s\nactual:   %s\n", message, expected, actual);
        return 1;
    }

    return 0;
}

static int test_legacy_body_golden_outputs(void) {
    char buffer[256];
    mochad_event_t event;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_PL;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE_UNIT;
    event.house = 'A';
    event.unit = 1;
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "PL selected house/unit event did not format");
    if (expect_text(buffer, "Rx PL HouseUnit: A1", "PL selected house/unit output changed"))
        return 1;

    event.function = "On";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "PL house/unit event did not format");
    if (expect_text(buffer, "Rx PL HouseUnit: A1 Func: On", "PL house/unit output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RF;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE_UNIT;
    event.house = 'A';
    event.unit = 1;
    event.function = "On";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "RF house/unit event did not format");
    if (expect_text(buffer, "Rx RF HouseUnit: A1 Func: On", "RF house/unit output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_TX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RF;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE;
    event.house = 'B';
    event.function = "Dim";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "RF house event did not format");
    if (expect_text(buffer, "Tx RF House: B Func: Dim", "RF house output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_PL;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE;
    event.house = 'C';
    event.function = "Bright";
    event.has_level = 1;
    event.level = 7;
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "PL dim/bright event did not format");
    if (expect_text(buffer, "Rx PL House: C Func: Bright(7)", "PL dim/bright output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_PL;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE_UNIT;
    event.house = 'D';
    event.unit = 4;
    event.function = "Ext code 1, data, control";
    event.has_extended_data = 1;
    event.extended_data = 0x21;
    event.extended_command = 0x31;
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "extended PL event did not format");
    if (expect_text(buffer,
                    "Rx PL HouseUnit: D4 Func: Ext code 1, data, control Data: 21 Command: 31",
                    "extended PL output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RFSEC;
    event.detail = MOCHAD_EVENT_DETAIL_RFSEC_SHORT;
    event.security_short_addr = 0xe2;
    event.function = "ARM_KR10A";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "short RFSEC event did not format");
    if (expect_text(buffer, "Rx RFSEC Addr: 0xE2 Func: ARM_KR10A", "short RFSEC output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RFSEC;
    event.detail = MOCHAD_EVENT_DETAIL_RFSEC_LONG;
    event.security_long_addr[0] = 0x01;
    event.security_long_addr[1] = 0x23;
    event.security_long_addr[2] = 0x45;
    event.function = "CONTACT_ALERT_MIN";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "long RFSEC event did not format");
    if (expect_text(buffer, "Rx RFSEC Addr: 01:23:45 Func: CONTACT_ALERT_MIN",
                    "long RFSEC output changed"))
        return 1;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_TX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RFCAM;
    event.detail = MOCHAD_EVENT_DETAIL_RFCAM;
    event.camera_command = "A CAMUP";
    if (mochad_event_format_legacy_body(&event, buffer, sizeof(buffer)) < 0)
        return expect(0, "RFCAM event did not format");
    return expect_text(buffer, "Tx RFCAM A CAMUP", "RFCAM output changed");
}

static int test_native_and_xmlsocket_framing(void) {
    char buffer[256];
    const char expected[] = "Rx RF HouseUnit: A2 Func: Off";
    mochad_event_t event;
    int length;

    memset(&event, 0, sizeof(event));
    event.direction = MOCHAD_EVENT_DIRECTION_RX;
    event.transport = MOCHAD_EVENT_TRANSPORT_RF;
    event.detail = MOCHAD_EVENT_DETAIL_HOUSE_UNIT;
    event.house = 'A';
    event.unit = 2;
    event.function = "Off";

    length = mochad_event_format_legacy_line(&event, buffer, sizeof(buffer));
    if (length < 0)
        return expect(0, "legacy line did not format");
    if (expect_text(buffer, "Rx RF HouseUnit: A2 Func: Off\n", "native TCP framing changed"))
        return 1;
    if (expect((size_t)length == strlen("Rx RF HouseUnit: A2 Func: Off\n"),
               "native TCP length changed"))
        return 1;

    length = mochad_event_format_openremote_line(&event, buffer, sizeof(buffer));
    if (length < 0)
        return expect(0, "OpenRemote line did not format");
    if (expect_text(buffer, "Rx RF HouseUnit: A2 Func: Off\n", "OpenRemote framing changed"))
        return 1;

    memset(buffer, 0x7f, sizeof(buffer));
    length = mochad_event_format_xmlsocket_line(&event, buffer, sizeof(buffer));
    if (length < 0)
        return expect(0, "XMLSocket line did not format");
    if (expect(memcmp(buffer, expected, strlen(expected)) == 0, "XMLSocket body changed"))
        return 1;
    if (expect(buffer[strlen(expected)] == '\0', "XMLSocket delimiter changed"))
        return 1;
    return expect((size_t)length == strlen(expected) + 1, "XMLSocket length changed");
}

int main(void) {
    if (test_legacy_body_golden_outputs())
        return 1;
    if (test_native_and_xmlsocket_framing())
        return 1;

    puts("PASS: mochad_event");
    return 0;
}

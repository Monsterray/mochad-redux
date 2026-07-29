#include "transport_evidence.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static size_t occurrences(const char *text, const char *needle) {
    size_t count = 0;

    while ((text = strstr(text, needle)) != NULL) {
        count++;
        text += strlen(needle);
    }
    return count;
}

static int test_command_attempt_and_receive_facts(void) {
    char json[16384];
    uint64_t command_id;
    mochad_transport_attempt attempt;

    mochad_transport_evidence_reset();
    if (expect(mochad_transport_evidence_tracks_command(" rf A1 on"),
               "RF command should be tracked") ||
        expect(!mochad_transport_evidence_tracks_command("health"),
               "diagnostic command should not be tracked"))
        return 1;

    command_id = mochad_transport_evidence_command_begin();
    mochad_transport_evidence_receive("tx", "rf", "decoded");
    attempt = mochad_transport_evidence_attempt_begin();
    mochad_transport_evidence_usb_queued(&attempt);
    mochad_transport_evidence_usb_submitted(&attempt);
    mochad_transport_evidence_usb_completed();
    mochad_transport_evidence_controller_acked();
    mochad_transport_evidence_attempt_terminal("succeeded", "controller_ack");
    mochad_transport_evidence_attempt_terminal("failed", "must_be_ignored");
    mochad_transport_evidence_command_finish(command_id);
    mochad_transport_evidence_receive("rx", "rf", "decoded");
    mochad_transport_evidence_duplicate("rx", "rf");

    if (expect(mochad_transport_evidence_json(json, sizeof(json)) > 0,
               "evidence JSON should fit") ||
        expect(strstr(json, "\"external_correlation\":\"unavailable\"") != NULL,
               "external correlation must be unavailable") ||
        expect(strstr(json, "\"kind\":\"redux.command_accepted\"") != NULL,
               "accepted command fact missing") ||
        expect(strstr(json, "\"subject\":\"x10-event\",\"command_id\":\"redux-c-1\","
                            "\"attempt_id\":null") != NULL,
               "transmit decode fact should carry the Redux-local command ID") ||
        expect(strstr(json, "\"kind\":\"redux.usb_submitted\"") != NULL,
               "USB submission fact missing") ||
        expect(strstr(json, "\"kind\":\"redux.controller_acked\"") != NULL,
               "controller ACK fact missing") ||
        expect(strstr(json, "\"decode_result\":\"duplicate_suppressed\"") != NULL,
               "duplicate decode result missing") ||
        expect(strstr(json, "\"duplicates_suppressed\":1") != NULL, "duplicate counter missing") ||
        expect(occurrences(json, "\"kind\":\"redux.attempt_terminal\"") == 1,
               "attempt must have one terminal fact"))
        return 1;

    return 0;
}

static int test_rejection_failure_and_ring_bound(void) {
    char json[16384];
    uint64_t command_id;
    mochad_transport_attempt attempt;
    unsigned int i;

    mochad_transport_evidence_reset();
    command_id = mochad_transport_evidence_command_begin();
    mochad_transport_evidence_command_finish(command_id);
    attempt = mochad_transport_evidence_attempt_begin();
    mochad_transport_evidence_usb_queue_failed(&attempt);

    for (i = 0; i < 40; i++)
        mochad_transport_evidence_receive("rx", "rf", "decoded");

    if (expect(mochad_transport_evidence_json(json, sizeof(json)) > 0,
               "bounded evidence JSON should fit") ||
        expect(strstr(json, "\"count\":32") != NULL, "ring count should stay bounded") ||
        expect(strstr(json, "\"dropped\":") != NULL, "ring drop count missing") ||
        expect(strstr(json, "\"reason\":\"queue_full\"") == NULL,
               "oldest queue failure should have been overwritten"))
        return 1;

    return 0;
}

static int test_failure_cancellation_and_timeout_facts(void) {
    char json[16384];
    mochad_transport_attempt attempt;

    mochad_transport_evidence_reset();
    attempt = mochad_transport_evidence_attempt_begin();
    mochad_transport_evidence_usb_submit_failed(&attempt, -5);

    attempt = mochad_transport_evidence_attempt_begin();
    mochad_transport_evidence_usb_submitted(&attempt);
    mochad_transport_evidence_usb_cancelled();
    mochad_transport_evidence_attempt_terminal("cancelled", "transfer_cancelled");

    attempt = mochad_transport_evidence_attempt_begin();
    mochad_transport_evidence_usb_submitted(&attempt);
    mochad_transport_evidence_usb_timed_out("ack_timeout");
    mochad_transport_evidence_usb_completed();
    mochad_transport_evidence_attempt_terminal("unknown", "ack_timeout");

    if (expect(mochad_transport_evidence_json(json, sizeof(json)) > 0,
               "failure evidence JSON should fit") ||
        expect(strstr(json, "\"kind\":\"redux.usb_failed\"") != NULL, "USB failure fact missing") ||
        expect(strstr(json, "\"kind\":\"redux.usb_cancelled\"") != NULL,
               "USB cancellation fact missing") ||
        expect(strstr(json, "\"kind\":\"redux.usb_timed_out\"") != NULL,
               "USB timeout fact missing") ||
        expect(strstr(json, "\"outcome\":\"unknown\"") != NULL, "unknown terminal outcome missing"))
        return 1;

    return 0;
}

int main(void) {
    if (test_command_attempt_and_receive_facts() || test_rejection_failure_and_ring_bound() ||
        test_failure_cancellation_and_timeout_facts())
        return 1;

    puts("PASS: transport_evidence");
    return 0;
}

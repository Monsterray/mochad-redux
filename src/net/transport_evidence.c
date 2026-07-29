#include "transport_evidence.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "version.h"

#define EVIDENCE_CAPACITY 32U

typedef struct {
    uint64_t sequence;
    time_t observed_at;
    const char *kind;
    const char *outcome;
    const char *reason;
    uint64_t command_id;
    uint64_t attempt_id;
    unsigned int attempt_number;
    const char *direction;
    const char *transport;
    const char *decode_result;
    unsigned long duplicate_count;
    int result_code;
} transport_fact;

static transport_fact Facts[EVIDENCE_CAPACITY];
static size_t FactStart;
static size_t FactCount;
static unsigned long DroppedFacts;
static unsigned long DuplicateCount;
static uint64_t NextSequence = 1;
static uint64_t NextCommandId = 1;
static uint64_t NextAttemptId = 1;
static uint64_t CurrentCommandId;
static unsigned int CurrentAttemptNumber;
static int CurrentCommandAccepted;
static mochad_transport_attempt ActiveAttempt;
static int ActiveAttemptTimedOut;

static uint64_t next_id(uint64_t *value) {
    uint64_t result = *value;

    if (result == 0)
        result = 1;
    *value = result + 1;
    if (*value == 0)
        *value = 1;
    return result;
}

static transport_fact *new_fact(const char *kind, const char *outcome) {
    size_t index;
    transport_fact *fact;

    if (FactCount < EVIDENCE_CAPACITY) {
        index = (FactStart + FactCount) % EVIDENCE_CAPACITY;
        FactCount++;
    } else {
        index = FactStart;
        FactStart = (FactStart + 1) % EVIDENCE_CAPACITY;
        DroppedFacts++;
    }

    fact = &Facts[index];
    memset(fact, 0, sizeof(*fact));
    fact->sequence = next_id(&NextSequence);
    fact->observed_at = time(NULL);
    fact->kind = kind;
    fact->outcome = outcome;
    return fact;
}

static void record_attempt(const char *kind, const char *outcome, const char *reason,
                           const mochad_transport_attempt *attempt, int result_code) {
    transport_fact *fact = new_fact(kind, outcome);

    fact->reason = reason;
    fact->result_code = result_code;
    if (attempt != NULL) {
        fact->command_id = attempt->command_id;
        fact->attempt_id = attempt->attempt_id;
        fact->attempt_number = attempt->attempt_number;
    }
}

static int first_token_is(const char *line, const char *expected) {
    size_t i = 0;

    while (isspace((unsigned char)*line))
        line++;
    while (expected[i] != '\0' && line[i] != '\0' && toupper((unsigned char)line[i]) == expected[i])
        i++;

    return expected[i] == '\0' && (line[i] == '\0' || isspace((unsigned char)line[i]));
}

void mochad_transport_evidence_reset(void) {
    memset(Facts, 0, sizeof(Facts));
    FactStart = 0;
    FactCount = 0;
    DroppedFacts = 0;
    DuplicateCount = 0;
    NextSequence = 1;
    NextCommandId = 1;
    NextAttemptId = 1;
    CurrentCommandId = 0;
    CurrentAttemptNumber = 0;
    CurrentCommandAccepted = 0;
    memset(&ActiveAttempt, 0, sizeof(ActiveAttempt));
    ActiveAttemptTimedOut = 0;
}

int mochad_transport_evidence_tracks_command(const char *line) {
    if (line == NULL)
        return 0;

    return first_token_is(line, "PL") || first_token_is(line, "RF") ||
           first_token_is(line, "RFSEC") || first_token_is(line, "RFCAM") ||
           first_token_is(line, "PT");
}

uint64_t mochad_transport_evidence_command_begin(void) {
    CurrentCommandId = next_id(&NextCommandId);
    CurrentAttemptNumber = 0;
    CurrentCommandAccepted = 0;
    return CurrentCommandId;
}

void mochad_transport_evidence_command_finish(uint64_t command_id) {
    transport_fact *fact;

    if (command_id == 0 || command_id != CurrentCommandId)
        return;

    if (!CurrentCommandAccepted) {
        fact = new_fact("redux.command_rejected", "failed");
        fact->command_id = command_id;
        fact->reason = "no_transport_work";
    }

    CurrentCommandId = 0;
    CurrentAttemptNumber = 0;
    CurrentCommandAccepted = 0;
}

mochad_transport_attempt mochad_transport_evidence_attempt_begin(void) {
    mochad_transport_attempt attempt;
    transport_fact *fact;
    int implicit_command = 0;

    if (CurrentCommandId == 0) {
        CurrentCommandId = next_id(&NextCommandId);
        CurrentAttemptNumber = 0;
        implicit_command = 1;
    }
    if (!CurrentCommandAccepted) {
        fact = new_fact("redux.command_accepted", "succeeded");
        fact->command_id = CurrentCommandId;
        CurrentCommandAccepted = 1;
    }

    CurrentAttemptNumber++;
    attempt.command_id = CurrentCommandId;
    attempt.attempt_id = next_id(&NextAttemptId);
    attempt.attempt_number = CurrentAttemptNumber;

    if (implicit_command) {
        CurrentCommandId = 0;
        CurrentAttemptNumber = 0;
        CurrentCommandAccepted = 0;
    }
    return attempt;
}

mochad_transport_attempt
mochad_transport_evidence_attempt_retry(const mochad_transport_attempt *previous) {
    mochad_transport_attempt attempt;

    attempt.command_id = previous->command_id;
    attempt.attempt_id = next_id(&NextAttemptId);
    attempt.attempt_number = previous->attempt_number + 1;
    return attempt;
}

void mochad_transport_evidence_usb_queued(const mochad_transport_attempt *attempt) {
    record_attempt("redux.usb_queued", "pending", NULL, attempt, 0);
}

void mochad_transport_evidence_usb_queue_failed(const mochad_transport_attempt *attempt) {
    record_attempt("redux.usb_failed", "failed", "queue_full", attempt, 0);
    record_attempt("redux.attempt_terminal", "failed", "queue_full", attempt, 0);
}

void mochad_transport_evidence_usb_submitted(const mochad_transport_attempt *attempt) {
    record_attempt("redux.usb_submitted", "pending", NULL, attempt, 0);
    ActiveAttempt = *attempt;
    ActiveAttemptTimedOut = 0;
}

void mochad_transport_evidence_usb_submit_failed(const mochad_transport_attempt *attempt,
                                                 int result_code) {
    record_attempt("redux.usb_failed", "failed", "submission_failed", attempt, result_code);
    record_attempt("redux.attempt_terminal", "failed", "submission_failed", attempt, result_code);
}

void mochad_transport_evidence_usb_completed(void) {
    record_attempt("redux.usb_completed", "succeeded", NULL, &ActiveAttempt, 0);
}

void mochad_transport_evidence_usb_failed(int result_code) {
    record_attempt("redux.usb_failed", "failed", "transfer_failed", &ActiveAttempt, result_code);
}

void mochad_transport_evidence_usb_cancelled(void) {
    record_attempt("redux.usb_cancelled", "cancelled", NULL, &ActiveAttempt, 0);
}

void mochad_transport_evidence_usb_timed_out(const char *reason) {
    if (ActiveAttempt.attempt_id == 0 || ActiveAttemptTimedOut)
        return;

    record_attempt("redux.usb_timed_out", "timed_out", reason, &ActiveAttempt, 0);
    ActiveAttemptTimedOut = 1;
}

void mochad_transport_evidence_controller_acked(void) {
    record_attempt("redux.controller_acked", "succeeded", "byte_0x55", &ActiveAttempt, 0);
}

void mochad_transport_evidence_attempt_terminal(const char *outcome, const char *reason) {
    if (ActiveAttempt.attempt_id == 0)
        return;

    record_attempt("redux.attempt_terminal", outcome, reason, &ActiveAttempt, 0);
    memset(&ActiveAttempt, 0, sizeof(ActiveAttempt));
}

void mochad_transport_evidence_attempt_terminal_for(const mochad_transport_attempt *attempt,
                                                    const char *outcome, const char *reason) {
    if (attempt == NULL || attempt->attempt_id == 0 ||
        attempt->attempt_id == ActiveAttempt.attempt_id)
        return;

    record_attempt("redux.attempt_terminal", outcome, reason, attempt, 0);
}

void mochad_transport_evidence_receive(const char *direction, const char *transport,
                                       const char *decode_result) {
    const char *outcome = "failed";
    transport_fact *fact;

    if (strcmp(decode_result, "decoded") == 0)
        outcome = "succeeded";
    else if (strcmp(decode_result, "unsupported") == 0)
        outcome = "unsupported";
    fact = new_fact("redux.receive_decoded", outcome);

    fact->direction = direction;
    fact->transport = transport;
    fact->decode_result = decode_result;
    fact->duplicate_count = DuplicateCount;
    fact->command_id = CurrentCommandId;
}

void mochad_transport_evidence_duplicate(const char *direction, const char *transport) {
    transport_fact *fact;

    DuplicateCount++;
    fact = new_fact("redux.receive_decoded", "succeeded");
    fact->direction = direction;
    fact->transport = transport;
    fact->decode_result = "duplicate_suppressed";
    fact->duplicate_count = DuplicateCount;
    fact->command_id = CurrentCommandId;
}

static int append(char *buffer, size_t buffer_len, size_t *used, const char *format, ...) {
    va_list args;
    int written;

    if (*used >= buffer_len)
        return -1;

    va_start(args, format);
    written = vsnprintf(buffer + *used, buffer_len - *used, format, args);
    va_end(args);
    if (written < 0 || (size_t)written >= buffer_len - *used)
        return -1;

    *used += (size_t)written;
    return 0;
}

static int append_nullable_id(char *buffer, size_t buffer_len, size_t *used, const char *name,
                              const char *prefix, uint64_t value) {
    if (value == 0)
        return append(buffer, buffer_len, used, "\"%s\":null", name);
    return append(buffer, buffer_len, used, "\"%s\":\"%s%llu\"", name, prefix,
                  (unsigned long long)value);
}

static int append_timestamp(char *buffer, size_t buffer_len, size_t *used, time_t observed_at) {
    char timestamp[32];
    struct tm utc;

    if (gmtime_r(&observed_at, &utc) == NULL ||
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &utc) == 0)
        return -1;
    return append(buffer, buffer_len, used, "%s", timestamp);
}

static int append_details(char *buffer, size_t buffer_len, size_t *used,
                          const transport_fact *fact) {
    int needs_comma = 0;

    if (append(buffer, buffer_len, used, "{") < 0)
        return -1;
#define DETAIL(format, ...)                                                                        \
    do {                                                                                           \
        if (append(buffer, buffer_len, used, needs_comma ? "," format : format, __VA_ARGS__) < 0)  \
            return -1;                                                                             \
        needs_comma = 1;                                                                           \
    } while (0)
    if (fact->reason)
        DETAIL("\"reason\":\"%s\"", fact->reason);
    if (fact->result_code != 0)
        DETAIL("\"result_code\":%d", fact->result_code);
    if (fact->direction)
        DETAIL("\"direction\":\"%s\"", fact->direction);
    if (fact->transport)
        DETAIL("\"transport\":\"%s\"", fact->transport);
    if (fact->decode_result)
        DETAIL("\"decode_result\":\"%s\"", fact->decode_result);
    if (fact->decode_result)
        DETAIL("\"duplicate_count\":%lu", fact->duplicate_count);
#undef DETAIL
    return append(buffer, buffer_len, used, "}");
}

static int append_fact(char *buffer, size_t buffer_len, size_t *used, const transport_fact *fact) {
    if (append(buffer, buffer_len, used,
               "{\"schema_version\":1,\"fact_id\":\"redux-%ld-%llu\","
               "\"kind\":\"%s\",\"emitter\":{\"repository\":\"mochad-redux\","
               "\"component\":\"transport\",\"version\":\"%s\",\"sha\":null},"
               "\"observed_at\":\"",
               (long)getpid(), (unsigned long long)fact->sequence, fact->kind,
               MOCHAD_REDUX_VERSION) < 0 ||
        append_timestamp(buffer, buffer_len, used, fact->observed_at) < 0 ||
        append(buffer, buffer_len, used, "\",\"outcome\":\"%s\",\"subject\":\"%s\",", fact->outcome,
               fact->decode_result ? "x10-event" : "controller") < 0 ||
        append_nullable_id(buffer, buffer_len, used, "command_id", "redux-c-", fact->command_id) <
            0 ||
        append(buffer, buffer_len, used, ",") < 0 ||
        append_nullable_id(buffer, buffer_len, used, "attempt_id", "redux-a-", fact->attempt_id) <
            0)
        return -1;

    if (fact->attempt_number == 0) {
        if (append(buffer, buffer_len, used,
                   ",\"attempt_number\":null,\"parent_fact_id\":null,\"details\":") < 0)
            return -1;
    } else if (append(buffer, buffer_len, used,
                      ",\"attempt_number\":%u,\"parent_fact_id\":null,\"details\":",
                      fact->attempt_number) < 0) {
        return -1;
    }

    if (append_details(buffer, buffer_len, used, fact) < 0)
        return -1;
    return append(buffer, buffer_len, used, "}");
}

int mochad_transport_evidence_json(char *buffer, size_t buffer_len) {
    size_t used = 0;
    size_t i;

    if (buffer == NULL || buffer_len == 0)
        return -1;

    if (append(buffer, buffer_len, &used,
               "{\"ok\":true,\"schema_version\":1,\"external_correlation\":"
               "\"unavailable\","
               "\"capacity\":%u,\"count\":%lu,\"dropped\":%lu,"
               "\"duplicates_suppressed\":%lu,\"facts\":[",
               EVIDENCE_CAPACITY, (unsigned long)FactCount, DroppedFacts, DuplicateCount) < 0)
        return -1;

    for (i = 0; i < FactCount; i++) {
        const transport_fact *fact = &Facts[(FactStart + i) % EVIDENCE_CAPACITY];

        if (i > 0 && append(buffer, buffer_len, &used, ",") < 0)
            return -1;
        if (append_fact(buffer, buffer_len, &used, fact) < 0)
            return -1;
    }

    if (append(buffer, buffer_len, &used, "]}") < 0)
        return -1;
    return (int)used;
}

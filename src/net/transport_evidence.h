#ifndef MOCHAD_TRANSPORT_EVIDENCE_H
#define MOCHAD_TRANSPORT_EVIDENCE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t command_id;
    uint64_t attempt_id;
    unsigned int attempt_number;
} mochad_transport_attempt;

void mochad_transport_evidence_reset(void);
int mochad_transport_evidence_tracks_command(const char *line);
uint64_t mochad_transport_evidence_command_begin(void);
void mochad_transport_evidence_command_finish(uint64_t command_id);

mochad_transport_attempt mochad_transport_evidence_attempt_begin(void);
mochad_transport_attempt
mochad_transport_evidence_attempt_retry(const mochad_transport_attempt *previous);
void mochad_transport_evidence_usb_queued(const mochad_transport_attempt *attempt);
void mochad_transport_evidence_usb_queue_failed(const mochad_transport_attempt *attempt);
void mochad_transport_evidence_usb_submitted(const mochad_transport_attempt *attempt);
void mochad_transport_evidence_usb_submit_failed(const mochad_transport_attempt *attempt,
                                                 int result_code);
void mochad_transport_evidence_usb_completed(void);
void mochad_transport_evidence_usb_failed(int result_code);
void mochad_transport_evidence_usb_cancelled(void);
void mochad_transport_evidence_usb_timed_out(const char *reason);
void mochad_transport_evidence_controller_acked(void);
void mochad_transport_evidence_attempt_terminal(const char *outcome, const char *reason);
void mochad_transport_evidence_attempt_terminal_for(const mochad_transport_attempt *attempt,
                                                    const char *outcome, const char *reason);

void mochad_transport_evidence_receive(const char *direction, const char *transport,
                                       const char *decode_result);
void mochad_transport_evidence_duplicate(const char *direction, const char *transport);

int mochad_transport_evidence_json(char *buffer, size_t buffer_len);

#endif

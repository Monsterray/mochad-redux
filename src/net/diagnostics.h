/*
 * JSON builders for additive TCP diagnostics.
 */

#ifndef MOCHAD_DIAGNOSTICS_H
#define MOCHAD_DIAGNOSTICS_H

#include <stddef.h>

#include "config.h"

typedef struct mochad_diag_runtime {
    unsigned long uptime_seconds;
    int usb_connected;
    const char *controller;
    int endpoints_ready;
    int transfers_ready;
    unsigned long clients_main;
    unsigned long clients_xml;
    unsigned long clients_openremote;
    unsigned long usb_out_completed;
    unsigned long usb_ack_received;
    unsigned long usb_ack_timeout;
    unsigned long usb_unexpected_one_byte;
    unsigned int max_clients;
    unsigned int next_client_id;
    const mochad_config *config;
} mochad_diag_runtime;

int mochad_diag_json_hello(char *buffer, size_t buffer_len, const char *upstream_base);
int mochad_diag_json_capabilities(char *buffer, size_t buffer_len, int raw_data);
int mochad_diag_json_health(char *buffer, size_t buffer_len, const char *upstream_base,
                            const mochad_diag_runtime *runtime);
int mochad_diag_json_clients(char *buffer, size_t buffer_len, const mochad_diag_runtime *runtime);
int mochad_diag_json_config(char *buffer, size_t buffer_len, const mochad_config *config);
int mochad_diag_json_version(char *buffer, size_t buffer_len, const char *upstream_base);

#endif

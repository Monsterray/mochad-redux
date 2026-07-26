/*
 * Central runtime configuration for mochad-redux.
 */

#ifndef MOCHAD_CONFIG_H
#define MOCHAD_CONFIG_H

#include <stddef.h>
#include <stdio.h>
#include <syslog.h>

#define MOCHAD_DEFAULT_BIND_ADDRESS "0.0.0.0"
#define MOCHAD_DEFAULT_SERVER_PORT 1099
#define MOCHAD_DEFAULT_XML_PORT 1100
#define MOCHAD_DEFAULT_OPENREMOTE_PORT 1101
#define MOCHAD_CONFIG_BIND_ADDRESS_LEN 128
#define MOCHAD_CONFIG_PATH_LEN 512

typedef enum {
    MOCHAD_DUAL_STACK_AUTO = 0,
    MOCHAD_DUAL_STACK_ENABLE,
    MOCHAD_DUAL_STACK_DISABLE
} mochad_dual_stack_policy;

typedef struct mochad_config {
    char bind_address[MOCHAD_CONFIG_BIND_ADDRESS_LEN];
    int server_port;
    int xml_port;
    int openremote_port;
    int xml_enabled;
    int openremote_enabled;
    int foreground;
    int raw_data;
    mochad_dual_stack_policy dual_stack;
    int log_level;
    int check_config;
    int print_config;
    int show_help;
    int show_version;
    char config_file[MOCHAD_CONFIG_PATH_LEN];
} mochad_config;

extern mochad_config MochadConfig;

void mochad_config_defaults(mochad_config *config);
int mochad_config_load(mochad_config *config, int argc, char **argv, char *error, size_t error_len);
int mochad_config_validate(const mochad_config *config, char *error, size_t error_len);
const char *mochad_config_dual_stack_name(mochad_dual_stack_policy policy);
const char *mochad_config_log_level_name(int level);
int mochad_config_snprint_json(char *buffer, size_t buffer_len, const mochad_config *config);
void mochad_config_print(FILE *stream, const mochad_config *config);

#endif

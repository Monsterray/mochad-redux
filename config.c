/*
 * Central runtime configuration for mochad-redux.
 */

#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

mochad_config MochadConfig;

static void set_error(char *error, size_t error_len, const char *message) {
    if (error_len == 0)
        return;

    snprintf(error, error_len, "%s", message);
}

static void set_error2(char *error, size_t error_len, const char *prefix, const char *value) {
    if (error_len == 0)
        return;

    snprintf(error, error_len, "%s%s", prefix, value);
}

static char *trim(char *value) {
    char *end;

    while (*value && isspace((unsigned char)*value))
        value++;

    end = value + strlen(value);
    while (end > value && isspace((unsigned char)end[-1]))
        end--;

    *end = '\0';
    return value;
}

static void strip_optional_quotes(char *value) {
    size_t len = strlen(value);

    if (len < 2)
        return;

    if ((value[0] == '"' && value[len - 1] == '"') ||
        (value[0] == '\'' && value[len - 1] == '\'')) {
        memmove(value, value + 1, len - 2);
        value[len - 2] = '\0';
    }
}

static void normalize_key(char *key) {
    char *src = key;
    char *dst = key;

    while (*src) {
        char ch = *src++;

        if (ch == '-')
            ch = '_';

        *dst++ = (char)tolower((unsigned char)ch);
    }

    *dst = '\0';

    if (strncmp(key, "mochad_", 7) == 0)
        memmove(key, key + 7, strlen(key + 7) + 1);
}

static int copy_string(char *dest, size_t dest_len, const char *value, char *error,
                       size_t error_len) {
    if (value == NULL || value[0] == '\0') {
        set_error(error, error_len, "value cannot be empty");
        return -1;
    }

    if (strlen(value) >= dest_len) {
        set_error(error, error_len, "value is too long");
        return -1;
    }

    snprintf(dest, dest_len, "%s", value);
    return 0;
}

static int parse_bool(const char *value, int *result) {
    char normalized[16];
    size_t i;
    size_t len;

    if (value == NULL)
        return -1;

    len = strlen(value);
    if (len >= sizeof(normalized))
        return -1;

    for (i = 0; i <= len; i++)
        normalized[i] = (char)tolower((unsigned char)value[i]);

    if (strcmp(normalized, "1") == 0 || strcmp(normalized, "true") == 0 ||
        strcmp(normalized, "yes") == 0 || strcmp(normalized, "on") == 0 ||
        strcmp(normalized, "enable") == 0 || strcmp(normalized, "enabled") == 0) {
        *result = 1;
        return 0;
    }

    if (strcmp(normalized, "0") == 0 || strcmp(normalized, "false") == 0 ||
        strcmp(normalized, "no") == 0 || strcmp(normalized, "off") == 0 ||
        strcmp(normalized, "disable") == 0 || strcmp(normalized, "disabled") == 0) {
        *result = 0;
        return 0;
    }

    return -1;
}

static int parse_port(const char *name, const char *value, int *port, char *error,
                      size_t error_len) {
    char *endptr = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(value, &endptr, 10);
    if (errno || endptr == value || *endptr != '\0' || parsed < 1 || parsed > 65535) {
        snprintf(error, error_len, "%s must be a TCP port from 1 to 65535: %s", name, value);
        return -1;
    }

    *port = (int)parsed;
    return 0;
}

static int parse_dual_stack(const char *value, mochad_dual_stack_policy *policy) {
    char normalized[24];
    size_t i;
    size_t len;

    len = strlen(value);
    if (len >= sizeof(normalized))
        return -1;

    for (i = 0; i <= len; i++) {
        char ch = value[i];

        if (ch == '-')
            ch = '_';

        normalized[i] = (char)tolower((unsigned char)ch);
    }

    if (strcmp(normalized, "auto") == 0) {
        *policy = MOCHAD_DUAL_STACK_AUTO;
        return 0;
    }

    if (strcmp(normalized, "1") == 0 || strcmp(normalized, "true") == 0 ||
        strcmp(normalized, "yes") == 0 || strcmp(normalized, "on") == 0 ||
        strcmp(normalized, "enable") == 0 || strcmp(normalized, "enabled") == 0) {
        *policy = MOCHAD_DUAL_STACK_ENABLE;
        return 0;
    }

    if (strcmp(normalized, "0") == 0 || strcmp(normalized, "false") == 0 ||
        strcmp(normalized, "no") == 0 || strcmp(normalized, "off") == 0 ||
        strcmp(normalized, "disable") == 0 || strcmp(normalized, "disabled") == 0 ||
        strcmp(normalized, "ipv6_only") == 0) {
        *policy = MOCHAD_DUAL_STACK_DISABLE;
        return 0;
    }

    return -1;
}

static int parse_log_level(const char *value, int *level) {
    char normalized[16];
    char *endptr = NULL;
    long parsed;
    size_t i;
    size_t len;

    errno = 0;
    parsed = strtol(value, &endptr, 10);
    if (!errno && endptr != value && *endptr == '\0' && parsed >= LOG_EMERG &&
        parsed <= LOG_DEBUG) {
        *level = (int)parsed;
        return 0;
    }

    len = strlen(value);
    if (len >= sizeof(normalized))
        return -1;

    for (i = 0; i <= len; i++)
        normalized[i] = (char)tolower((unsigned char)value[i]);

    if (strcmp(normalized, "emerg") == 0 || strcmp(normalized, "emergency") == 0)
        *level = LOG_EMERG;
    else if (strcmp(normalized, "alert") == 0)
        *level = LOG_ALERT;
    else if (strcmp(normalized, "crit") == 0 || strcmp(normalized, "critical") == 0)
        *level = LOG_CRIT;
    else if (strcmp(normalized, "err") == 0 || strcmp(normalized, "error") == 0)
        *level = LOG_ERR;
    else if (strcmp(normalized, "warning") == 0 || strcmp(normalized, "warn") == 0)
        *level = LOG_WARNING;
    else if (strcmp(normalized, "notice") == 0)
        *level = LOG_NOTICE;
    else if (strcmp(normalized, "info") == 0)
        *level = LOG_INFO;
    else if (strcmp(normalized, "debug") == 0)
        *level = LOG_DEBUG;
    else
        return -1;

    return 0;
}

static int apply_key_value(mochad_config *config, const char *raw_key, const char *raw_value,
                           char *error, size_t error_len) {
    char key[64];
    char value[256];
    int bool_value;

    if (strlen(raw_key) >= sizeof(key)) {
        set_error2(error, error_len, "configuration key too long: ", raw_key);
        return -1;
    }

    if (strlen(raw_value) >= sizeof(value)) {
        set_error2(error, error_len, "configuration value too long for key: ", raw_key);
        return -1;
    }

    snprintf(key, sizeof(key), "%s", raw_key);
    snprintf(value, sizeof(value), "%s", raw_value);
    normalize_key(key);
    strip_optional_quotes(value);

    if (strcmp(key, "bind") == 0 || strcmp(key, "bind_address") == 0)
        return copy_string(config->bind_address, sizeof(config->bind_address), value, error,
                           error_len);

    if (strcmp(key, "port") == 0 || strcmp(key, "server_port") == 0 ||
        strcmp(key, "main_port") == 0)
        return parse_port(raw_key, value, &config->server_port, error, error_len);

    if (strcmp(key, "xml_port") == 0)
        return parse_port(raw_key, value, &config->xml_port, error, error_len);

    if (strcmp(key, "openremote_port") == 0)
        return parse_port(raw_key, value, &config->openremote_port, error, error_len);

    if (strcmp(key, "xml_enabled") == 0 || strcmp(key, "enable_xml") == 0) {
        if (parse_bool(value, &bool_value) < 0) {
            set_error2(error, error_len, "invalid boolean for ", raw_key);
            return -1;
        }
        config->xml_enabled = bool_value;
        return 0;
    }

    if (strcmp(key, "openremote_enabled") == 0 || strcmp(key, "enable_openremote") == 0) {
        if (parse_bool(value, &bool_value) < 0) {
            set_error2(error, error_len, "invalid boolean for ", raw_key);
            return -1;
        }
        config->openremote_enabled = bool_value;
        return 0;
    }

    if (strcmp(key, "foreground") == 0) {
        if (parse_bool(value, &bool_value) < 0) {
            set_error2(error, error_len, "invalid boolean for ", raw_key);
            return -1;
        }
        config->foreground = bool_value;
        return 0;
    }

    if (strcmp(key, "raw_data") == 0) {
        if (parse_bool(value, &bool_value) < 0) {
            set_error2(error, error_len, "invalid boolean for ", raw_key);
            return -1;
        }
        config->raw_data = bool_value;
        return 0;
    }

    if (strcmp(key, "dual_stack") == 0) {
        if (parse_dual_stack(value, &config->dual_stack) < 0) {
            set_error2(error, error_len, "dual_stack must be auto, enable, or disable: ", value);
            return -1;
        }
        return 0;
    }

    if (strcmp(key, "log_level") == 0) {
        if (parse_log_level(value, &config->log_level) < 0) {
            set_error2(error, error_len, "invalid log_level: ", value);
            return -1;
        }
        return 0;
    }

    set_error2(error, error_len, "unknown configuration key: ", raw_key);
    return -1;
}

static int load_config_file(mochad_config *config, const char *path, char *error,
                            size_t error_len) {
    FILE *file;
    char line[512];
    unsigned long line_number = 0;

    if (path == NULL || path[0] == '\0')
        return 0;

    file = fopen(path, "r");
    if (file == NULL) {
        snprintf(error, error_len, "could not open config file %s: %s", path, strerror(errno));
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char *hash;
        char *equals;
        char *key;
        char *value;

        line_number++;
        hash = strchr(line, '#');
        if (hash != NULL)
            *hash = '\0';

        key = trim(line);
        if (*key == '\0')
            continue;

        equals = strchr(key, '=');
        if (equals == NULL) {
            snprintf(error, error_len, "invalid config file line %lu: expected key=value",
                     line_number);
            fclose(file);
            return -1;
        }

        *equals = '\0';
        value = trim(equals + 1);
        key = trim(key);

        if (apply_key_value(config, key, value, error, error_len) < 0) {
            char detail[256];

            snprintf(detail, sizeof(detail), "invalid config file line %lu: %s", line_number,
                     error);
            set_error(error, error_len, detail);
            fclose(file);
            return -1;
        }
    }

    if (ferror(file)) {
        snprintf(error, error_len, "error reading config file %s", path);
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

static int apply_env_value(mochad_config *config, const char *env_name, const char *key,
                           char *error, size_t error_len) {
    const char *value = getenv(env_name);

    if (value == NULL || value[0] == '\0')
        return 0;

    if (apply_key_value(config, key, value, error, error_len) < 0) {
        char detail[256];

        snprintf(detail, sizeof(detail), "invalid %s: %s", env_name, error);
        set_error(error, error_len, detail);
        return -1;
    }

    return 0;
}

static int apply_environment(mochad_config *config, char *error, size_t error_len) {
    if (apply_env_value(config, "MOCHAD_BIND", "bind", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_BIND_ADDRESS", "bind", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_PORT", "port", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_SERVER_PORT", "port", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_XML_ENABLED", "xml_enabled", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_XML_PORT", "xml_port", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_OPENREMOTE_ENABLED", "openremote_enabled", error,
                        error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_OPENREMOTE_PORT", "openremote_port", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_FOREGROUND", "foreground", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_RAW_DATA", "raw_data", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_DUAL_STACK", "dual_stack", error, error_len) < 0)
        return -1;
    if (apply_env_value(config, "MOCHAD_LOG_LEVEL", "log_level", error, error_len) < 0)
        return -1;

    return 0;
}

static int require_arg(int argc, char **argv, int *index, char *error, size_t error_len) {
    if (*index + 1 >= argc) {
        snprintf(error, error_len, "%s requires an argument", argv[*index]);
        return -1;
    }

    (*index)++;
    return 0;
}

static int find_config_file_arg(int argc, char **argv, char *path, size_t path_len, char *error,
                                size_t error_len) {
    int i;
    const char *env_path = getenv("MOCHAD_CONFIG");

    path[0] = '\0';

    if (env_path != NULL && env_path[0] != '\0') {
        if (copy_string(path, path_len, env_path, error, error_len) < 0)
            return -1;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (copy_string(path, path_len, argv[i], error, error_len) < 0)
                return -1;
        }
    }

    return 0;
}

static int apply_cli(mochad_config *config, int argc, char **argv, char *error, size_t error_len) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--foreground") == 0) {
            config->foreground = 1;
        } else if (strcmp(argv[i], "--background") == 0) {
            config->foreground = 0;
        } else if (strcmp(argv[i], "--raw-data") == 0) {
            config->raw_data = 1;
        } else if (strcmp(argv[i], "--no-raw-data") == 0) {
            config->raw_data = 0;
        } else if (strcmp(argv[i], "--bind") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "bind", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--port") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "port", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--enable-xml") == 0) {
            config->xml_enabled = 1;
        } else if (strcmp(argv[i], "--disable-xml") == 0) {
            config->xml_enabled = 0;
        } else if (strcmp(argv[i], "--xml-port") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "xml_port", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--enable-openremote") == 0) {
            config->openremote_enabled = 1;
        } else if (strcmp(argv[i], "--disable-openremote") == 0) {
            config->openremote_enabled = 0;
        } else if (strcmp(argv[i], "--openremote-port") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "openremote_port", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--dual-stack") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "dual_stack", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--log-level") == 0) {
            if (require_arg(argc, argv, &i, error, error_len) < 0)
                return -1;
            if (apply_key_value(config, "log_level", argv[i], error, error_len) < 0)
                return -1;
        } else if (strcmp(argv[i], "--check-config") == 0) {
            config->check_config = 1;
        } else if (strcmp(argv[i], "--print-config") == 0) {
            config->print_config = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            config->show_version = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            config->show_help = 1;
        } else {
            set_error2(error, error_len, "unknown option ", argv[i]);
            return -1;
        }
    }

    return 0;
}

void mochad_config_defaults(mochad_config *config) {
    memset(config, 0, sizeof(*config));
    snprintf(config->bind_address, sizeof(config->bind_address), "%s", MOCHAD_DEFAULT_BIND_ADDRESS);
    config->server_port = MOCHAD_DEFAULT_SERVER_PORT;
    config->xml_port = MOCHAD_DEFAULT_XML_PORT;
    config->openremote_port = MOCHAD_DEFAULT_OPENREMOTE_PORT;
    config->xml_enabled = 1;
    config->openremote_enabled = 1;
    config->foreground = 0;
    config->raw_data = 0;
    config->dual_stack = MOCHAD_DUAL_STACK_AUTO;
    config->log_level = LOG_DEBUG;
}

int mochad_config_load(mochad_config *config, int argc, char **argv, char *error,
                       size_t error_len) {
    char config_path[MOCHAD_CONFIG_PATH_LEN];

    mochad_config_defaults(config);

    if (find_config_file_arg(argc, argv, config_path, sizeof(config_path), error, error_len) < 0)
        return -1;

    if (config_path[0] != '\0') {
        if (copy_string(config->config_file, sizeof(config->config_file), config_path, error,
                        error_len) < 0)
            return -1;
        if (load_config_file(config, config_path, error, error_len) < 0)
            return -1;
    }

    if (apply_environment(config, error, error_len) < 0)
        return -1;

    if (apply_cli(config, argc, argv, error, error_len) < 0)
        return -1;

    return mochad_config_validate(config, error, error_len);
}

int mochad_config_validate(const mochad_config *config, char *error, size_t error_len) {
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    char portbuf[16];
    int rc;

    if (config->bind_address[0] == '\0') {
        set_error(error, error_len, "bind address cannot be empty");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
    snprintf(portbuf, sizeof(portbuf), "%d", config->server_port);
    rc = getaddrinfo(config->bind_address, portbuf, &hints, &result);
    if (rc != 0) {
        snprintf(error, error_len, "bind address must be a numeric IPv4 or IPv6 address: %s",
                 config->bind_address);
        return -1;
    }
    freeaddrinfo(result);

    if ((config->xml_enabled && config->server_port == config->xml_port) ||
        (config->openremote_enabled && config->server_port == config->openremote_port) ||
        (config->xml_enabled && config->openremote_enabled &&
         config->xml_port == config->openremote_port)) {
        set_error(error, error_len, "enabled listener ports must be distinct");
        return -1;
    }

    if (config->log_level < LOG_EMERG || config->log_level > LOG_DEBUG) {
        set_error(error, error_len, "log_level is out of syslog range");
        return -1;
    }

    return 0;
}

const char *mochad_config_dual_stack_name(mochad_dual_stack_policy policy) {
    switch (policy) {
    case MOCHAD_DUAL_STACK_AUTO:
        return "auto";
    case MOCHAD_DUAL_STACK_ENABLE:
        return "enable";
    case MOCHAD_DUAL_STACK_DISABLE:
        return "disable";
    }

    return "unknown";
}

const char *mochad_config_log_level_name(int level) {
    switch (level) {
    case LOG_EMERG:
        return "emergency";
    case LOG_ALERT:
        return "alert";
    case LOG_CRIT:
        return "critical";
    case LOG_ERR:
        return "error";
    case LOG_WARNING:
        return "warning";
    case LOG_NOTICE:
        return "notice";
    case LOG_INFO:
        return "info";
    case LOG_DEBUG:
        return "debug";
    }

    return "unknown";
}

int mochad_config_snprint_json(char *buffer, size_t buffer_len, const mochad_config *config) {
    int written;

    written = snprintf(buffer, buffer_len,
                       "{\"ok\":true,\"bind_address\":\"%s\","
                       "\"foreground\":%s,\"raw_data\":%s,"
                       "\"dual_stack\":\"%s\",\"log_level\":\"%s\","
                       "\"listeners\":{\"main\":{\"enabled\":true,\"port\":%d},"
                       "\"xml\":{\"enabled\":%s,\"port\":%d},"
                       "\"openremote\":{\"enabled\":%s,\"port\":%d}}}",
                       config->bind_address, config->foreground ? "true" : "false",
                       config->raw_data ? "true" : "false",
                       mochad_config_dual_stack_name(config->dual_stack),
                       mochad_config_log_level_name(config->log_level), config->server_port,
                       config->xml_enabled ? "true" : "false", config->xml_port,
                       config->openremote_enabled ? "true" : "false", config->openremote_port);

    if (written < 0 || (size_t)written >= buffer_len)
        return -1;

    return written;
}

void mochad_config_print(FILE *stream, const mochad_config *config) {
    char buffer[1024];

    if (mochad_config_snprint_json(buffer, sizeof(buffer), config) < 0) {
        fprintf(stream, "{\"ok\":false,\"error\":\"config output too large\"}\n");
        return;
    }

    fprintf(stream, "%s\n", buffer);
}

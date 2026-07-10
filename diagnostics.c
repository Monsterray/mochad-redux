/*
 * JSON builders for additive TCP diagnostics.
 */

#include "diagnostics.h"

#include <stdarg.h>
#include <stdio.h>

#include "version.h"

static const char *json_bool(int value)
{
    return value ? "true" : "false";
}

static const char *safe_string(const char *value)
{
    return value ? value : "";
}

static int checked_snprintf(char *buffer, size_t buffer_len,
        const char *format, ...)
{
    va_list args;
    int written;

    if (buffer_len == 0)
        return -1;

    va_start(args, format);
    written = vsnprintf(buffer, buffer_len, format, args);
    va_end(args);

    if (written < 0 || (size_t)written >= buffer_len)
        return -1;

    return written;
}

int mochad_diag_json_hello(char *buffer, size_t buffer_len,
        const char *upstream_base)
{
    return checked_snprintf(buffer, buffer_len,
            "{\"ok\":true,\"daemon\":\"mochad-redux\",\"version\":\"%s\","
            "\"upstream_base\":\"%s\",\"diagnostics\":true}",
            MOCHAD_REDUX_VERSION, safe_string(upstream_base));
}

int mochad_diag_json_capabilities(char *buffer, size_t buffer_len,
        int raw_data)
{
    return checked_snprintf(buffer, buffer_len,
            "{\"ok\":true,\"commands\":[\"hello\",\"capabilities\","
            "\"health\",\"clients\",\"config\",\"version\"],"
            "\"legacy_commands\":[\"pl\",\"rf\",\"rfsec\",\"rfcam\",\"pt\","
            "\"rftopl\",\"rftorf\",\"st\",\"getstatus\",\"getstatussec\"],"
            "\"json\":true,\"single_line\":true,\"raw_data\":%s}",
            json_bool(raw_data));
}

int mochad_diag_json_health(char *buffer, size_t buffer_len,
        const char *upstream_base, const mochad_diag_runtime *runtime)
{
    if (runtime == NULL || runtime->config == NULL)
        return -1;

    return checked_snprintf(buffer, buffer_len,
            "{\"ok\":true,\"version\":\"%s\",\"upstream_base\":\"%s\","
            "\"uptime_seconds\":%lu,\"usb_connected\":%s,"
            "\"controller\":\"%s\",\"endpoints_ready\":%s,"
            "\"transfers_ready\":%s,\"clients_total\":%lu,"
            "\"bind_address\":\"%s\",\"listeners\":{\"main\":{\"enabled\":true,"
            "\"port\":%d},\"xml\":{\"enabled\":%s,\"port\":%d},"
            "\"openremote\":{\"enabled\":%s,\"port\":%d}}}",
            MOCHAD_REDUX_VERSION, safe_string(upstream_base),
            runtime->uptime_seconds, json_bool(runtime->usb_connected),
            safe_string(runtime->controller), json_bool(runtime->endpoints_ready),
            json_bool(runtime->transfers_ready),
            runtime->clients_main + runtime->clients_xml +
            runtime->clients_openremote,
            runtime->config->bind_address, runtime->config->server_port,
            json_bool(runtime->config->xml_enabled), runtime->config->xml_port,
            json_bool(runtime->config->openremote_enabled),
            runtime->config->openremote_port);
}

int mochad_diag_json_clients(char *buffer, size_t buffer_len,
        const mochad_diag_runtime *runtime)
{
    if (runtime == NULL)
        return -1;

    return checked_snprintf(buffer, buffer_len,
            "{\"ok\":true,\"clients\":{\"main\":%lu,\"xml\":%lu,"
            "\"openremote\":%lu,\"total\":%lu},\"max_clients\":%u,"
            "\"next_client_id\":%u}",
            runtime->clients_main, runtime->clients_xml,
            runtime->clients_openremote,
            runtime->clients_main + runtime->clients_xml +
            runtime->clients_openremote,
            runtime->max_clients, runtime->next_client_id);
}

int mochad_diag_json_config(char *buffer, size_t buffer_len,
        const mochad_config *config)
{
    if (config == NULL)
        return -1;

    return mochad_config_snprint_json(buffer, buffer_len, config);
}

int mochad_diag_json_version(char *buffer, size_t buffer_len,
        const char *upstream_base)
{
    return checked_snprintf(buffer, buffer_len,
            "{\"ok\":true,\"daemon\":\"mochad-redux\",\"version\":\"%s\","
            "\"upstream_base\":\"%s\"}",
            MOCHAD_REDUX_VERSION, safe_string(upstream_base));
}

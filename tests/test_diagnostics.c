#include "diagnostics.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int print_json(const char *name, int result, const char *json) {
    if (result < 0) {
        fprintf(stderr, "FAIL: %s JSON did not fit\n", name);
        return 1;
    }

    puts(json);
    return 0;
}

int main(void) {
    char json[2048];
    mochad_config config;
    mochad_diag_runtime runtime;

    mochad_config_defaults(&config);
    snprintf(config.bind_address, sizeof(config.bind_address), "%s", "127.0.0.1");
    config.raw_data = 1;

    memset(&runtime, 0, sizeof(runtime));
    runtime.uptime_seconds = 42;
    runtime.usb_connected = 1;
    runtime.controller = "CM19A";
    runtime.endpoints_ready = 1;
    runtime.transfers_ready = 1;
    runtime.clients_main = 2;
    runtime.clients_xml = 1;
    runtime.clients_openremote = 0;
    runtime.max_clients = 32;
    runtime.next_client_id = 4;
    runtime.config = &config;

    if (print_json("hello", mochad_diag_json_hello(json, sizeof(json), "mochad 0.1.18"), json))
        return 1;
    if (print_json("capabilities",
                   mochad_diag_json_capabilities(json, sizeof(json), config.raw_data), json))
        return 1;
    if (print_json("health", mochad_diag_json_health(json, sizeof(json), "mochad 0.1.18", &runtime),
                   json))
        return 1;
    if (print_json("clients", mochad_diag_json_clients(json, sizeof(json), &runtime), json))
        return 1;
    if (print_json("config", mochad_diag_json_config(json, sizeof(json), &config), json))
        return 1;
    if (print_json("version", mochad_diag_json_version(json, sizeof(json), "mochad 0.1.18"), json))
        return 1;

    if (expect(mochad_diag_json_hello(json, 8, "mochad 0.1.18") == -1,
               "small diagnostic buffer should fail"))
        return 1;

    return 0;
}

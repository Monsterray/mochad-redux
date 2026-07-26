#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void clear_mochad_env(void) {
    static const char *names[] = {
        "MOCHAD_CONFIG",          "MOCHAD_BIND",
        "MOCHAD_BIND_ADDRESS",    "MOCHAD_PORT",
        "MOCHAD_SERVER_PORT",     "MOCHAD_XML_ENABLED",
        "MOCHAD_XML_PORT",        "MOCHAD_OPENREMOTE_ENABLED",
        "MOCHAD_OPENREMOTE_PORT", "MOCHAD_FOREGROUND",
        "MOCHAD_RAW_DATA",        "MOCHAD_DUAL_STACK",
        "MOCHAD_LOG_LEVEL",
    };
    size_t i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        unsetenv(names[i]);
}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int test_defaults(void) {
    char arg0[] = "test_config";
    char *argv[] = {arg0};
    char error[256];
    mochad_config config;

    clear_mochad_env();
    if (mochad_config_load(&config, 1, argv, error, sizeof(error)) < 0) {
        fprintf(stderr, "FAIL: defaults rejected: %s\n", error);
        return 1;
    }

    if (expect(strcmp(config.bind_address, MOCHAD_DEFAULT_BIND_ADDRESS) == 0,
               "default bind address changed"))
        return 1;
    if (expect(config.server_port == MOCHAD_DEFAULT_SERVER_PORT, "default main port changed"))
        return 1;
    if (expect(config.xml_enabled == 1 && config.openremote_enabled == 1,
               "legacy listeners should default to enabled"))
        return 1;
    return expect(config.raw_data == 0, "raw data should default off");
}

static int write_config_file(char *path, size_t path_len) {
    FILE *file;
    int fd;

    snprintf(path, path_len, "/tmp/mochad-config-test.XXXXXX");
    fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return -1;
    }

    file = fdopen(fd, "w");
    if (file == NULL) {
        perror("fdopen");
        close(fd);
        return -1;
    }

    fputs("bind=127.0.0.1\n", file);
    fputs("port=1200\n", file);
    fputs("xml_enabled=false\n", file);
    fputs("openremote_enabled=true\n", file);
    fputs("openremote_port=1201\n", file);
    fputs("raw_data=true\n", file);
    fputs("dual_stack=disable\n", file);
    fputs("log_level=info\n", file);
    fclose(file);
    return 0;
}

static int test_precedence(void) {
    char path[128];
    char error[256];
    mochad_config config;
    char arg0[] = "test_config";
    char arg1[] = "--config";
    char arg3[] = "--port";
    char arg4[] = "1400";
    char *argv[] = {
        arg0, arg1, path, arg3, arg4,
    };

    clear_mochad_env();
    if (write_config_file(path, sizeof(path)) < 0)
        return 1;

    if (setenv("MOCHAD_PORT", "1300", 1) < 0) {
        perror("setenv");
        unlink(path);
        return 1;
    }

    if (mochad_config_load(&config, 5, argv, error, sizeof(error)) < 0) {
        fprintf(stderr, "FAIL: precedence rejected: %s\n", error);
        unlink(path);
        return 1;
    }

    unlink(path);

    if (expect(config.server_port == 1400, "CLI should override environment and config file"))
        return 1;
    if (expect(config.xml_enabled == 0, "config file boolean should apply"))
        return 1;
    if (expect(config.raw_data == 1, "config file raw_data should apply"))
        return 1;
    if (expect(config.dual_stack == MOCHAD_DUAL_STACK_DISABLE,
               "config file dual_stack should apply"))
        return 1;
    return expect(config.log_level == LOG_INFO, "config file log_level should apply");
}

static int test_invalid_port(void) {
    char arg0[] = "test_config";
    char arg1[] = "--port";
    char arg2[] = "70000";
    char *argv[] = {arg0, arg1, arg2};
    char error[256];
    mochad_config config;

    clear_mochad_env();
    if (mochad_config_load(&config, 3, argv, error, sizeof(error)) == 0)
        return expect(0, "invalid port should fail validation");

    return expect(strstr(error, "TCP port") != NULL, "invalid port error should be clear");
}

int main(void) {
    if (test_defaults())
        return 1;
    if (test_precedence())
        return 1;
    if (test_invalid_port())
        return 1;

    puts("PASS: config");
    return 0;
}

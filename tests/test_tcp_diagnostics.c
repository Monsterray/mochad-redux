#include "diagnostics.h"
#include "socket_io.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_json_line(int fd, const char *json)
{
    if (send_all(fd, json, strlen(json)) < 0)
        return -1;
    return send_all(fd, "\n", 1);
}

static int build_listener(int *port)
{
    struct sockaddr_in address;
    socklen_t address_len;
    int fd;
    int one = 1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        close(fd);
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    address.sin_port = 0;

    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(fd);
        return -1;
    }

    if (listen(fd, 1) < 0) {
        close(fd);
        return -1;
    }

    address_len = sizeof(address);
    if (getsockname(fd, (struct sockaddr *)&address, &address_len) < 0) {
        close(fd);
        return -1;
    }

    *port = ntohs(address.sin_port);
    return fd;
}

static int connect_client(int port)
{
    struct sockaddr_in address;
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    address.sin_port = htons((unsigned short)port);

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static int emit_diagnostic_lines(int fd)
{
    char json[2048];
    mochad_config config;
    mochad_diag_runtime runtime;

    mochad_config_defaults(&config);
    snprintf(config.bind_address, sizeof(config.bind_address), "%s",
            "127.0.0.1");

    memset(&runtime, 0, sizeof(runtime));
    runtime.uptime_seconds = 7;
    runtime.controller = "none";
    runtime.max_clients = 32;
    runtime.next_client_id = 1;
    runtime.config = &config;

    if (mochad_diag_json_hello(json, sizeof(json), "mochad 0.1.18") < 0 ||
            send_json_line(fd, json) < 0)
        return -1;
    if (mochad_diag_json_capabilities(json, sizeof(json), config.raw_data) < 0 ||
            send_json_line(fd, json) < 0)
        return -1;
    if (mochad_diag_json_health(json, sizeof(json), "mochad 0.1.18",
                &runtime) < 0 || send_json_line(fd, json) < 0)
        return -1;
    if (mochad_diag_json_config(json, sizeof(json), &config) < 0 ||
            send_json_line(fd, json) < 0)
        return -1;
    if (mochad_diag_json_version(json, sizeof(json), "mochad 0.1.18") < 0 ||
            send_json_line(fd, json) < 0)
        return -1;

    return 0;
}

int main(void)
{
    char buffer[4096];
    int listen_fd;
    int client_fd;
    int server_fd;
    int port;
    ssize_t bytes;

    listen_fd = build_listener(&port);
    if (listen_fd < 0) {
        perror("build_listener");
        return 1;
    }

    client_fd = connect_client(port);
    if (client_fd < 0) {
        perror("connect_client");
        close(listen_fd);
        return 1;
    }

    server_fd = accept(listen_fd, NULL, NULL);
    if (server_fd < 0) {
        perror("accept");
        close(client_fd);
        close(listen_fd);
        return 1;
    }

    if (emit_diagnostic_lines(server_fd) < 0) {
        perror("emit_diagnostic_lines");
        close(server_fd);
        close(client_fd);
        close(listen_fd);
        return 1;
    }

    close(server_fd);
    close(listen_fd);

    while ((bytes = read(client_fd, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, (size_t)bytes, stdout) != (size_t)bytes) {
            perror("fwrite");
            close(client_fd);
            return 1;
        }
    }

    close(client_fd);
    if (bytes < 0) {
        perror("read");
        return 1;
    }

    return 0;
}

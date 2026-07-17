#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "decode.h"
#include "encode.h"
#include "global.h"
#include "socket_io.h"
#include "x10state.h"
#include "x10_write.h"

int Cm19a = 1;
int PollTimeOut = -1;
int raw_data = 0;
unsigned short RfToPl16 = 0xFFFF;
unsigned short RfToRf16 = 0;

static unsigned char last_usb_write[8];
static size_t last_usb_write_len = 0;
static int usb_write_calls = 0;

int _dbprintf(const char *fmt, ...)
{
    (void)fmt;
    return 0;
}

int send_all(int fd, const void *buffer, size_t length)
{
    (void)fd;
    (void)buffer;
    return (int)length;
}

int send_all_with_sender(
        int fd,
        const void *buffer,
        size_t length,
        mochad_send_func sender,
        void *context)
{
    (void)fd;
    (void)buffer;
    (void)sender;
    (void)context;
    return (int)length;
}

int statusprintf(int fd, const char *fmt, ...)
{
    (void)fd;
    (void)fmt;
    return 0;
}

int sockprintf(int fd, const char *fmt, ...)
{
    (void)fd;
    (void)fmt;
    return 0;
}

int or20client(int fd)
{
    (void)fd;
    return 0;
}

int del_client(int fd)
{
    (void)fd;
    return 0;
}

void hexdump(void *p, size_t len)
{
    (void)p;
    (void)len;
}

int x10_write(unsigned char *buf, size_t len)
{
    usb_write_calls++;
    last_usb_write_len = len;
    memcpy(last_usb_write, buf, len);
    return (int)len;
}

void cm15a_decode_plc(int fd, unsigned char *buf, size_t len)
{
    (void)fd;
    (void)buf;
    (void)len;
}

void cm15a_decode_rf(int fd, unsigned char *buf, unsigned int len)
{
    (void)fd;
    (void)buf;
    (void)len;
}

void cm15a_decode(int fd, unsigned char *buf, unsigned int len)
{
    (void)fd;
    (void)buf;
    (void)len;
}

const char *findSecEventName(unsigned char secev)
{
    (void)secev;
    return "";
}

const char *findSecRemoteKeyName(unsigned char secev)
{
    (void)secev;
    return "";
}

int findCamRemoteCommand(const char *keyname)
{
    (void)keyname;
    return -1;
}

void hua_sec_init(void) {}
void hua_sec_event(unsigned char *secaddr, unsigned int funcint,
        unsigned int secaddr8)
{
    (void)secaddr;
    (void)funcint;
    (void)secaddr8;
}
void hua_add(int house, int unit) { (void)house; (void)unit; }
void hua_func_all_on(int house) { (void)house; }
void hua_func_all_off(int house) { (void)house; }
void hua_func_on(int house) { (void)house; }
void hua_func_off(int house) { (void)house; }
void hua_show(int fd) { (void)fd; }
unsigned char hua_getstatus(int house, int unit)
{
    (void)house;
    (void)unit;
    return 0;
}
unsigned char hua_getstatus_xdim(int house, int unit)
{
    (void)house;
    (void)unit;
    return 0;
}
void hua_setstatus_xdim(int house, int unit, int xdim)
{
    (void)house;
    (void)unit;
    (void)xdim;
}
int hua_getstatus_sec(int rf8bitaddr, unsigned long rfaddr)
{
    (void)rf8bitaddr;
    (void)rfaddr;
    return 0;
}

int mochad_diag_hello(int fd) { (void)fd; return 0; }
int mochad_diag_capabilities(int fd) { (void)fd; return 0; }
int mochad_diag_health(int fd) { (void)fd; return 0; }
int mochad_diag_clients(int fd) { (void)fd; return 0; }
int mochad_diag_config(int fd) { (void)fd; return 0; }
int mochad_diag_version(int fd) { (void)fd; return 0; }

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int test_rf_a2_on_cm19a_encoding(void)
{
    char command[] = "rf A2 on";
    unsigned char expected[] = {0x20, 0x60, 0x9f, 0x10, 0xef};

    processcommandline(-1, command);

    if (expect(usb_write_calls == 1,
                "rf A2 on should submit one USB write"))
        return 1;
    if (expect(last_usb_write_len == sizeof(expected),
                "rf A2 on CM19A write length mismatch"))
        return 1;
    return expect(memcmp(last_usb_write, expected, sizeof(expected)) == 0,
            "rf A2 on CM19A bytes changed");
}

int main(void)
{
    if (test_rf_a2_on_cm19a_encoding())
        return 1;

    puts("PASS: encode_rf");
    return 0;
}

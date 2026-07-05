/*
 * Copyright 2010-2011 Brian Uechi <buasst@gmail.com>
 *
 * This file is part of mochad.
 *
 * mochad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mochad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mochad.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * TCP gateway to X10 CM15A X10 RF and PL controller and CM19A RF controller.
 * Decode data from CM15A/CM19A ignoring macros and timers. This driver treats
 * the CM15A as a transceiver. The CM15A macros, timers, and real-time clock
 * (RTC) are ignored. In fact, the CM15A memory should be cleared using
 * ActiveHome Pro (AHP) before using the CM15A is with this driver. Batteries
 * are not necessary because the RTC is not used. The CM15A RF to PL converter
 * should be disabled for all house codes using AHP.  The CM19A does not
 * supports macros, timers, or RTC so it can be used as-is.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

/**** system log ****/
#include <syslog.h>

/**** ioctl ****/
#include <sys/ioctl.h>

/* Multiple On-line Controllers Home Automation Daemon */
#define DAEMON_NAME "mochad"
#define REDUX_VERSION "mochad-redux v0.3.0"

#define LEVEL LOG_INFO // was originally LOG_EMERG

/**** socket ****/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "global.h"
#include "encode.h"

#define DEFAULT_BIND_ADDRESS "0.0.0.0"
#define DEFAULT_SERVER_PORT (1099)
#define DEFAULT_XML_PORT (1100)
#define DEFAULT_OPENREMOTE_PORT (1101)
#define MAXCLISOCKETS   (32)
#define MAXSOCKETS      (1+MAXCLISOCKETS)
				/* first socket=listen socket, 32 client sockets */
#define USB_FDS         (10)    /* libusb file descriptors */

static struct pollfd Clients[(3*MAXSOCKETS)+USB_FDS];

/* Client sockets */
static struct pollfd Clientsocks[MAXCLISOCKETS];
static struct pollfd Clientxmlsocks[MAXCLISOCKETS];
static struct pollfd Clientor20socks[MAXCLISOCKETS];
static cm15a_encode_state_t Clientstates[MAXCLISOCKETS];
static cm15a_encode_state_t Clientxmlstates[MAXCLISOCKETS];
static cm15a_encode_state_t Clientor20states[MAXCLISOCKETS];
static unsigned int Clientids[MAXCLISOCKETS];
static unsigned int Clientxmlids[MAXCLISOCKETS];
static unsigned int Clientor20ids[MAXCLISOCKETS];

static size_t NClients;     /* # of valid entries in Clientsocks     */
static size_t NxmlClients;  /* # of valid entries in Clientxmlsocks  */
static size_t Nor20Clients; /* # of valid entries in Clientor20socks */
static unsigned int NextClientId = 1;
static const char *BindAddress = DEFAULT_BIND_ADDRESS;
static int ServerPort = DEFAULT_SERVER_PORT;
static int XmlPort = DEFAULT_XML_PORT;
static int OpenRemotePort = DEFAULT_OPENREMOTE_PORT;
static int XmlEnabled = 1;
static int OpenRemoteEnabled = 1;
static time_t StartTime = 0;

/**** USB usblib 1.0 ****/

#include <libusb-1.0/libusb.h>
uint8_t InEndpoint, OutEndpoint;

static struct libusb_device_handle *Devh        = NULL;
static struct libusb_transfer *IntrOut_transfer = NULL;
static struct libusb_transfer *IntrIn_transfer  = NULL;

static unsigned char IntrOutBuf[8];
static unsigned char IntrInBuf[8];

extern int raw_data;

/*
 * Like printf but print to socket without date/time stamp.
 * Used to send back result of getstatus command.
 */
int statusprintf(int fd, const char *fmt, ...)
{
    va_list args;
    char buf[1024];
    int buflen;

    va_start(args,fmt);
    buflen = vsnprintf(buf, sizeof(buf)-2, fmt, args);
    va_end(args);
    return send(fd, buf, buflen, MSG_NOSIGNAL);
}

static unsigned long uptime_seconds(void)
{
    time_t now;

    if (StartTime == 0)
        return 0;

    now = time(NULL);
    if (now < StartTime)
        return 0;

    return (unsigned long)(now - StartTime);
}

static const char *controller_model(void)
{
    if (!Devh)
        return "none";

    return Cm19a ? "CM19A" : "CM15A";
}

static int usb_connected(void)
{
    return Devh != NULL;
}

static int endpoints_ready(void)
{
    return InEndpoint != 0 && OutEndpoint != 0;
}

static int transfers_ready(void)
{
    return IntrIn_transfer != NULL && IntrOut_transfer != NULL;
}

static size_t total_clients(void)
{
    return NClients + NxmlClients + Nor20Clients;
}

int mochad_diag_hello(int fd)
{
    return statusprintf(fd,
            "{\"ok\":true,\"daemon\":\"mochad-redux\",\"version\":\"%s\","
            "\"upstream_base\":\"%s\",\"diagnostics\":true}\n",
            REDUX_VERSION, PACKAGE_STRING);
}

int mochad_diag_capabilities(int fd)
{
    return statusprintf(fd,
            "{\"ok\":true,\"commands\":[\"hello\",\"capabilities\","
            "\"health\",\"clients\",\"version\"],"
            "\"legacy_commands\":[\"pl\",\"rf\",\"rfsec\",\"rfcam\",\"pt\","
            "\"rftopl\",\"rftorf\",\"st\",\"getstatus\",\"getstatussec\"],"
            "\"json\":true,\"single_line\":true,\"raw_data\":%s}\n",
            raw_data ? "true" : "false");
}

int mochad_diag_health(int fd)
{
    return statusprintf(fd,
            "{\"ok\":true,\"version\":\"%s\",\"upstream_base\":\"%s\","
            "\"uptime_seconds\":%lu,\"usb_connected\":%s,"
            "\"controller\":\"%s\",\"endpoints_ready\":%s,"
            "\"transfers_ready\":%s,\"clients_total\":%lu,"
            "\"bind_address\":\"%s\",\"listeners\":{\"main\":{\"enabled\":true,"
            "\"port\":%d},\"xml\":{\"enabled\":%s,\"port\":%d},"
            "\"openremote\":{\"enabled\":%s,\"port\":%d}}}\n",
            REDUX_VERSION, PACKAGE_STRING, uptime_seconds(),
            usb_connected() ? "true" : "false", controller_model(),
            endpoints_ready() ? "true" : "false",
            transfers_ready() ? "true" : "false",
            (unsigned long)total_clients(), BindAddress, ServerPort,
            XmlEnabled ? "true" : "false", XmlPort,
            OpenRemoteEnabled ? "true" : "false", OpenRemotePort);
}

int mochad_diag_clients(int fd)
{
    return statusprintf(fd,
            "{\"ok\":true,\"clients\":{\"main\":%lu,\"xml\":%lu,"
            "\"openremote\":%lu,\"total\":%lu},\"max_clients\":%d,"
            "\"next_client_id\":%u}\n",
            (unsigned long)NClients, (unsigned long)NxmlClients,
            (unsigned long)Nor20Clients, (unsigned long)total_clients(),
            MAXCLISOCKETS, NextClientId);
}

int mochad_diag_version(int fd)
{
    return statusprintf(fd,
            "{\"ok\":true,\"daemon\":\"mochad-redux\",\"version\":\"%s\","
            "\"upstream_base\":\"%s\"}\n",
            REDUX_VERSION, PACKAGE_STRING);
}

static int xmlclient(int fd)
{
    int i;
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (fd == Clientxmlsocks[i].fd) return 1;
    }
    return 0;
}

/* Return 0 if the socket fd is not an OpenRemote 2.0 client.
 * Else return 1. OR clients connect to OpenRemotePort, default 1101, so that is
 * used.
 *
 */
int or20client(int fd)
{
    struct sockaddr_storage locl;
    socklen_t locllen;
    unsigned short port;

    locllen = sizeof(locl);
    if (getsockname(fd, (struct sockaddr *)&locl, &locllen) < 0) {
        dbprintf("getsockname -1/%d\n", errno);
        return 0;
    }
    if (locl.ss_family == AF_INET) {
        port = ntohs(((struct sockaddr_in *)&locl)->sin_port);
    }
    else if (locl.ss_family == AF_INET6) {
        port = ntohs(((struct sockaddr_in6 *)&locl)->sin6_port);
    }
    else {
        dbprintf("locl family %d\n", locl.ss_family);
        return 0;
    }
    dbprintf("locl port %d\n", port);
    return (port == OpenRemotePort);
}

/*
 * Like printf but prefix each line with date/time stamp.
 * If fd == -1, send to all socket clients else send only to fd.
 */
int sockprintf(int fd, const char *fmt, ...)
{
    va_list args;
    char buf[1024];
    char *aLine;
    int len, buflen;
    time_t now;
    struct tm local_tm;
    int i;
    int bytesOut;

    aLine = buf;
    now = time(NULL);
    len = strftime(aLine, sizeof(buf), "%m/%d %T ", localtime_r(&now, &local_tm));
    va_start(args,fmt);
    buflen = vsnprintf(aLine+len, sizeof(buf)-len, fmt, args);
    va_end(args);
    buflen += len;
    if (fd != -1) {
        if (xmlclient(fd) && (aLine[buflen-1] == '\n')) {
            aLine[buflen-1] = '\0';
        }
        return send(fd, aLine, buflen, MSG_NOSIGNAL);
    }

    /* Send to all socket clients */
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if ((fd = Clientsocks[i].fd) > 0) {
            dbprintf("%s i %d fd %d\n", __func__, i, fd);
            bytesOut = send(fd, aLine, buflen, MSG_NOSIGNAL);
            dbprintf("bytesOut %d\n", bytesOut);
            if (bytesOut != buflen)
                dbprintf("%s: %d/%d\n", __func__, bytesOut, errno);
        }
    }
    /* Replace trialing newline with NUL if present. 
     * This assumes newline only at end of buffer.
     */
    if (aLine[buflen-1] == '\n') {
        aLine[buflen-1] = '\0';
    }
    /* Send to all xml socket clients */
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if ((fd = Clientxmlsocks[i].fd) > 0) {
            dbprintf("%s i %d fd %d\n", __func__, i, fd);
            /* NOTE: Send xml including trailing NUL '\0' */
            bytesOut = send(fd, aLine, buflen, MSG_NOSIGNAL);
            dbprintf("bytesOut %d\n", bytesOut);
            if (bytesOut != buflen)
                dbprintf("%s: %d/%d\n", __func__, bytesOut, errno);
        }
    }
    return buflen;
}

static void _hexdump(void *p, size_t len, char *outbuf, size_t outlen)
{
    unsigned char *ptr = (unsigned char*) p;
    size_t l, used = 0;

    if ((len == 0) || (outlen == 0)) return;
    if (len > ((outlen - 1) / 3))
        l = (outlen - 1) / 3;
    else
        l = len;
    while (l--) {
        snprintf(outbuf + used, outlen - used, "%02X ", *ptr++);
        used += 3;
    }
}

void hexdump(void *p, size_t len)
{
    char buf[(3*100)+1];

    _hexdump(p, len, buf, sizeof(buf));
    puts(buf);
}

void sockhexdump(int fd, void *p, size_t len)
{
    char buf[(3*100)+1];

    _hexdump(p, len, buf, sizeof(buf));
    sockprintf(fd, "%s\n", buf);
}

// Output Raw data with header for parsing by misterhouse
void mh_sockhexdump(int fd, void *p, size_t len)
{
    char buf[(3*100)+1];

    _hexdump(p, len, buf, sizeof(buf));
    sockprintf(fd, "Raw data received: %s\n", buf);
}


static volatile sig_atomic_t Do_exit = 0;
static volatile sig_atomic_t Exit_signal = 0;
static int Reattach = 0;

static unsigned int next_client_id(void)
{
    if (NextClientId == 0)
        NextClientId = 1;
    return NextClientId++;
}

static void init_client(void)
{
    int i;

    for (i = 0; i < MAXCLISOCKETS; i++) {
        Clientsocks[i].fd = Clientxmlsocks[i].fd = Clientor20socks[i].fd = -1;
        Clientids[i] = Clientxmlids[i] = Clientor20ids[i] = 0;
        cm15a_encode_state_init(&Clientstates[i]);
        cm15a_encode_state_init(&Clientxmlstates[i]);
        cm15a_encode_state_init(&Clientor20states[i]);
    }
    NClients = NxmlClients = Nor20Clients = 0;
}

/* Add new socket client */
static int add_client(int fd)
{
    int i;

    dbprintf("add_client(%d)\n", fd);
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientsocks[i].fd == -1) {
            Clientsocks[i].fd = fd;
            Clientsocks[i].events = POLLIN;
            Clientsocks[i].revents = 0;
            Clientids[i] = next_client_id();
            cm15a_encode_state_init(&Clientstates[i]);
            NClients++;
            dbprintf("add_client: i %d NClients %d\n", i, NClients);
            syslog(LOG_NOTICE, "[CLIENT] client id=%u connected type=main fd=%d",
                    Clientids[i], fd);
            return 0;
        }
    }
    dbprintf("max clients exceeded %d\n", i);
    syslog(LOG_INFO, "[CLIENT] rejected main client fd=%d: maximum clients reached",
            fd);
    return -1;
}

/* Add new flashxml socket client */
static int add_xmlclient(int fd)
{
    int i;

    dbprintf("add_xmlclient(%d)\n", fd);
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientxmlsocks[i].fd == -1) {
            Clientxmlsocks[i].fd = fd;
            Clientxmlsocks[i].events = POLLIN;
            Clientxmlsocks[i].revents = 0;
            Clientxmlids[i] = next_client_id();
            cm15a_encode_state_init(&Clientxmlstates[i]);
            NxmlClients++;
            dbprintf("add_xmlclient: i %d NxmlClients %d\n", i, NxmlClients);
            syslog(LOG_NOTICE, "[CLIENT] client id=%u connected type=xml fd=%d",
                    Clientxmlids[i], fd);
            return 0;
        }
    }
    dbprintf("max XML clients exceeded %d\n", i);
    syslog(LOG_INFO, "[CLIENT] rejected XML client fd=%d: maximum clients reached",
            fd);
    return -1;
}

/* Add new or20 socket client */
static int add_or20client(int fd)
{
    int i;

    dbprintf("add_or20client(%d)\n", fd);
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientor20socks[i].fd == -1) {
            Clientor20socks[i].fd = fd;
            Clientor20socks[i].events = POLLIN;
            Clientor20socks[i].revents = 0;
            Clientor20ids[i] = next_client_id();
            cm15a_encode_state_init(&Clientor20states[i]);
            Nor20Clients++;
            dbprintf("add_or20client: i %d Nor20Clients %d\n", i, Nor20Clients);
            syslog(LOG_NOTICE,
                    "[CLIENT] client id=%u connected type=openremote fd=%d",
                    Clientor20ids[i], fd);
            return 0;
        }
    }
    dbprintf("max OR20 clients exceeded %d\n", i);
    syslog(LOG_INFO,
            "[CLIENT] rejected OpenRemote client fd=%d: maximum clients reached",
            fd);
    return -1;
}

static cm15a_encode_state_t *client_encode_state(int fd)
{
    int i;

    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientsocks[i].fd == fd) return &Clientstates[i];
        if (Clientxmlsocks[i].fd == fd) return &Clientxmlstates[i];
        if (Clientor20socks[i].fd == fd) return &Clientor20states[i];
    }
    return NULL;
}

static unsigned int client_id_for_fd(int fd)
{
    int i;

    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientsocks[i].fd == fd) return Clientids[i];
        if (Clientxmlsocks[i].fd == fd) return Clientxmlids[i];
        if (Clientor20socks[i].fd == fd) return Clientor20ids[i];
    }
    return 0;
}

static void log_accept_result(const char *name, int fd)
{
    /* errno is meaningful only when accept() fails. Logging it after a
     * successful accept shows stale values from earlier syscalls.
     */
    if (fd < 0) {
        dbprintf("%s accept failed errno %d\n", name, errno);
        syslog(LOG_INFO, "[CLIENT] accept failed type=%s errno=%d", name, errno);
    }
    else {
        dbprintf("%s accept fd %d\n", name, fd);
    }
}

/* Delete socket client */
int del_client(int fd)
{
    int i;

    dbprintf("del_client(%d)\n", fd);
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientsocks[i].fd == fd) {
            syslog(LOG_NOTICE, "[CLIENT] client id=%u disconnected type=main fd=%d",
                    Clientids[i], fd);
            shutdown(fd, SHUT_RDWR);
            close(fd);
            Clientsocks[i].fd = -1;
            Clientids[i] = 0;
            cm15a_encode_state_init(&Clientstates[i]);
            NClients--;
            dbprintf("del_client: i %d NClients %d\n", i, NClients);
            return 0;
        }
        if (Clientxmlsocks[i].fd == fd) {
            syslog(LOG_NOTICE, "[CLIENT] client id=%u disconnected type=xml fd=%d",
                    Clientxmlids[i], fd);
            shutdown(fd, SHUT_RDWR);
            close(fd);
            Clientxmlsocks[i].fd = -1;
            Clientxmlids[i] = 0;
            cm15a_encode_state_init(&Clientxmlstates[i]);
            NxmlClients--;
            dbprintf("del_client: i %d NxmlClients %d\n", i, NxmlClients);
            return 0;
        }
        if (Clientor20socks[i].fd == fd) {
            syslog(LOG_NOTICE,
                    "[CLIENT] client id=%u disconnected type=openremote fd=%d",
                    Clientor20ids[i], fd);
            shutdown(fd, SHUT_RDWR);
            close(fd);
            Clientor20socks[i].fd = -1;
            Clientor20ids[i] = 0;
            cm15a_encode_state_init(&Clientor20states[i]);
            Nor20Clients--;
            dbprintf("del_client: i %d Nor20Clients %d\n", i, Nor20Clients);
            return 0;
        }
    }
    dbprintf("del_client:fd not found %d\n", fd);
    return -1;
}

/* Copy socket client records to array */
static int copy_clients(struct pollfd *Clients)
{
    int i;

    dbprintf("copy_clients\n");
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if (Clientsocks[i].fd != -1) {
            *Clients++ = Clientsocks[i];
        }
        if (Clientxmlsocks[i].fd != -1) {
            *Clients++ = Clientxmlsocks[i];
        }
        if (Clientor20socks[i].fd != -1) {
            *Clients++ = Clientor20socks[i];
        }
    }
    dbprintf("copy_clients %d\n", NClients+NxmlClients+Nor20Clients);
    return NClients+NxmlClients+Nor20Clients;
}
/* Client sockets */

#include "x10state.h"
#include "x10_write.h"
#include "decode.h"

/*
** This is interesting, it might be a good idea to create a text file which
** can be read in at startup (hmm, need to think that through)
*/

struct binarydata {
    size_t binlength;
    unsigned char bindata[8];
};

static const struct binarydata initcm15abinary[] = {
#if 0
    {8, {0x9b,0x00,0x5b,0x09,0x50,0x90,0x60,0x02}},
    {8, {0x9b,0x00,0x5b,0x09,0x50,0x90,0x60,0x02}},
    {8, {0xbb,0x00,0x00,0x05,0x00,0x14,0x20,0x28}},
    {1, {0x8b}},
    {3, {0xdb,0x1f,0xf0}},
    {3, {0xdb,0x20,0x00}},
    {3, {0xab,0xde,0xaf}},
    {1, {0x8b}},
    {3, {0xab,0x00,0x00}},
#endif
    {0}
};

static const struct binarydata initcm19abinary[] = {
    {8, {0x80,0x05,0x1b,0x14,0x28,0x20,0x24,0x29}},
    {2, {0x83,0x03}},
    {8, {0x84,0x37,0x02,0x60,0x00,0x00,0x00,0x00}},
    {8, {0x80,0x01,0x00,0x14,0x20,0x24,0x28,0x29}},
    {3, {0x83,0x02,0x0f}},
    {8, {0x83,0x37,0x02,0x60,0x00,0x00,0x00,0x00}},
    {5, {0x20,0x34,0xcb,0x58,0xa7}},
    {8, {0x80,0x05,0x01,0x14,0x20,0x24,0x28,0x29}},
    {0}
};


static void initcm1Xa(const struct binarydata *p)
{
    dbprintf("initcm1Xa\n");
    while (p->binlength) {
        x10_write((unsigned char *)p->bindata, p->binlength);
        p++;
    }
}

/*
** Find CM15A or CM19A. The EU versions (CM15Pro and CM19Pro) have the same
** vendor and product IDs, respectively.
*/

static int find_cm15a(struct libusb_device_handle **devhptr)
{
    int r;

    Cm19a = 0;
    *devhptr = libusb_open_device_with_vid_pid(NULL,  0x0bc7, 0x0001);
    if (!*devhptr) {
        *devhptr = libusb_open_device_with_vid_pid(NULL,  0x0bc7, 0x0002);
        if (!*devhptr) {
            syslog(LEVEL,
                    "[USB] CM15A/CM19A not found; in Docker, verify /dev/bus/usb is mapped and the container has USB permissions");
            return -EIO;
        }
        Cm19a = 1;
    }
    r = libusb_claim_interface(*devhptr, 0);
    if (r == 0) {
        syslog(LOG_NOTICE, "[USB] controller found model=%s",
                (Cm19a) ? "CM19A" : "CM15A");
        return 0;
    }
    syslog(LEVEL,
            "[USB] claim interface failed rc=%d; check permissions, Docker USB passthrough, or kernel drivers",
            r);
    r = libusb_kernel_driver_active(*devhptr, 0);
    if (r < 0) {
        syslog(LEVEL, "[USB] kernel driver check failed rc=%d", r);
        return -EIO;
    }
    syslog(LOG_NOTICE, "[USB] kernel driver active=%d; trying detach", r);
    r = libusb_detach_kernel_driver(*devhptr, 0);
    if (r < 0) {
        syslog(LEVEL,
                "[USB] kernel driver detach failed rc=%d; check drivers such as ati_remote",
                r);
        return -EIO;
    }
    Reattach = 1;
    r = libusb_claim_interface(*devhptr, 0);
    if (r < 0) {
        syslog(LEVEL, "[USB] claim interface failed after detach rc=%d", r);
        return -EIO;
    }
    syslog(LOG_NOTICE, "[USB] controller found model=%s",
            (Cm19a) ? "CM19A" : "CM15A");
    return 0;
}

/* Find the in and out endpoint address in the device descriptors.
 * This is required by newer CM19A that have changed endpoint addresses.
 */
static int get_endpoint_address(libusb_device_handle *devh, uint8_t *inendpt, uint8_t *outendpt)
{
    int r;
    struct libusb_config_descriptor *config = NULL;
    const struct libusb_interface *interfaces;
    const struct libusb_interface_descriptor *interface_desc;
    const struct libusb_endpoint_descriptor *endpoint_desc;
    struct libusb_device *uDevice;
    struct libusb_device_descriptor desc;
    int i, j, k;

    *inendpt = 0;
    *outendpt = 0;

    uDevice = libusb_get_device(devh);
    if (!uDevice) return -ENODEV;
    r = libusb_get_device_descriptor(uDevice, &desc);
    if (r < 0) return r;

    r = libusb_get_active_config_descriptor(uDevice, &config);
    if (r < 0) return r;
    if (!config) return -ENODEV;
    interfaces = config->interface;
    for (i = 0; i < config->bNumInterfaces; i++) {
        interface_desc = interfaces->altsetting;
        for (j = 0; j < interfaces->num_altsetting; j++) {
            endpoint_desc = interface_desc->endpoint;
            for (k = 0; k < interface_desc->bNumEndpoints; k++) {
                if (endpoint_desc->bEndpointAddress & 0x80) {
                    *inendpt = endpoint_desc->bEndpointAddress;
                }
                else {
                    *outendpt = endpoint_desc->bEndpointAddress;
                }
                endpoint_desc++;
            }
            interface_desc++;
        }
        interfaces++;
    }
    libusb_free_config_descriptor(config);

    if (!*inendpt || !*outendpt) return -ENODEV;
    return 0;
}

static void IntrOut_cb(struct libusb_transfer *transfer)
{
    /* dbprintf("IntrOut callback len %d\n", transfer->actual_length); */
}

static void IntrIn_cb(struct libusb_transfer *transfer)
{
#if 0
    int fd, i;
#endif

    if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
        dbprintf("IntrIn transfer status %d?\n", transfer->status);
        Do_exit = 2;
        libusb_free_transfer(transfer);
        IntrIn_transfer = NULL;
        return;
    }

    /* dbprintf("IntrIn callback len %d ", transfer->actual_length); */
    /* hexdump(transfer->buffer, transfer->actual_length); */

/*        if ((transfer->actual_length == 1) && (*transfer->buffer == 0x55)) {  */
    if (transfer->actual_length == 1) {
        send_next_x10out();
    }

#if 0
    /* Incoming USB data is sent to all sockets */
    for (i = 0; i < MAXCLISOCKETS; i++) {
        if ((fd = Clientsocks[i].fd) > 0) {
            cm15a_decode(fd, transfer->buffer, transfer->actual_length);
        }
    }
#else
    cm15a_decode(-1, transfer->buffer, transfer->actual_length);
#endif
    if (libusb_submit_transfer(IntrIn_transfer) < 0) {
        syslog(LEVEL,
                "[USB] interrupt input transfer resubmit failed; shutting down");
        Do_exit = 2;
    }
}

static int start_transfers(void)
{
    int r;

    r = libusb_submit_transfer(IntrIn_transfer);
    if (r < 0) {
        syslog(LEVEL,
                "[USB] interrupt input transfer submit failed rc=%d; controller is not ready",
                r);
        return r;
    }
    return 0;
}

static int do_init(void)
{
    // set clock?

    return 0;
}

static int alloc_transfers(void)
{
    IntrIn_transfer = libusb_alloc_transfer(0);
    if (!IntrIn_transfer) {
        syslog(LEVEL, "[USB] interrupt input transfer allocation failed");
        return -ENOMEM;
    }
    libusb_fill_interrupt_transfer(IntrIn_transfer, Devh, InEndpoint, 
            IntrInBuf, sizeof(IntrInBuf), IntrIn_cb, NULL, 0);

    IntrOut_transfer = libusb_alloc_transfer(0);
    if (!IntrOut_transfer) {
        syslog(LEVEL, "[USB] interrupt output transfer allocation failed");
        libusb_free_transfer(IntrIn_transfer);
        IntrIn_transfer = NULL;
        return -ENOMEM;
    }
    return 0;
}

int write_usb(unsigned char *buf, size_t len)
{
    int r, i;

    dbprintf("usb len %lu ", (unsigned long)len);
    hexdump(buf, len);
    if (len > sizeof(IntrOutBuf)) {
        dbprintf("usb write too long %lu/%lu\n", (unsigned long)len,
                (unsigned long)sizeof(IntrOutBuf));
        return -EINVAL;
    }
    memcpy(IntrOutBuf, buf, len);
    libusb_fill_interrupt_transfer(IntrOut_transfer, Devh, OutEndpoint, 
            IntrOutBuf, len, IntrOut_cb, NULL, 0);
    r = libusb_submit_transfer(IntrOut_transfer);
    if (r < 0) {
        libusb_cancel_transfer(IntrOut_transfer);
        i = 100;
        while (IntrOut_transfer && i--)
            if (libusb_handle_events(NULL) < 0)
                break;
        return r;
    }
    return 0;
}

static void sighandler(int signum)
{
    Exit_signal = signum;
    Do_exit = 1;	
}

static const char *signal_name(int signum)
{
    switch (signum) {
        case SIGINT:
            return "SIGINT";
        case SIGTERM:
            return "SIGTERM";
        case SIGQUIT:
            return "SIGQUIT";
        default:
            return "unknown signal";
    }
}

static int parse_port_option(const char *name, const char *value, int *port)
{
    char *endptr = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(value, &endptr, 10);
    if (errno || endptr == value || *endptr != '\0' ||
            parsed < 1 || parsed > 65535) {
        fprintf(stderr, "%s must be a TCP port from 1 to 65535: %s\n",
                name, value);
        return -1;
    }
    *port = (int)parsed;
    return 0;
}

static int validate_runtime_config(void)
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    char portbuf[16];
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
    snprintf(portbuf, sizeof(portbuf), "%d", ServerPort);
    rc = getaddrinfo(BindAddress, portbuf, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "--bind must be a numeric IPv4 or IPv6 address: %s\n",
                BindAddress);
        return -1;
    }
    freeaddrinfo(result);
    if ((XmlEnabled && ServerPort == XmlPort) ||
            (OpenRemoteEnabled && ServerPort == OpenRemotePort) ||
            (XmlEnabled && OpenRemoteEnabled && XmlPort == OpenRemotePort)) {
        fprintf(stderr,
                "enabled listener ports must be distinct\n");
        return -1;
    }
    return 0;
}

static const char *socket_family_name(int family)
{
    if (family == AF_INET)
        return "ipv4";
    if (family == AF_INET6)
        return "ipv6";
    return "unknown";
}

static int create_listener(const char *name, int port)
{
    struct addrinfo hints;
    struct addrinfo *results = NULL;
    struct addrinfo *candidate;
    char portbuf[16];
    int fd = -1;
    int rc;
    int on = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;
    snprintf(portbuf, sizeof(portbuf), "%d", port);

    rc = getaddrinfo(BindAddress, portbuf, &hints, &results);
    if (rc != 0) {
        syslog(LEVEL, "[TCP] invalid bind address=%s port=%d error=%s",
                BindAddress, port, gai_strerror(rc));
        return -1;
    }

    for (candidate = results; candidate != NULL; candidate = candidate->ai_next) {
        int dual_stack_enabled = 0;
        int dual_stack_failed = 0;
        const char *dual_stack = "not_applicable";

        fd = socket(candidate->ai_family, candidate->ai_socktype,
                candidate->ai_protocol);
        if (fd < 0) {
            syslog(LEVEL,
                    "[TCP] socket failed listener=%s address=%s port=%d family=%s errno=%d",
                    name, BindAddress, port,
                    socket_family_name(candidate->ai_family), errno);
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on,
                    sizeof(on)) < 0) {
            syslog(LEVEL,
                    "[TCP] setsockopt failed listener=%s address=%s port=%d family=%s option=SO_REUSEADDR errno=%d",
                    name, BindAddress, port,
                    socket_family_name(candidate->ai_family), errno);
            close(fd);
            fd = -1;
            continue;
        }

        if (candidate->ai_family == AF_INET6) {
            int v6only = 0;

            if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &v6only,
                        sizeof(v6only)) < 0) {
                dual_stack_failed = 1;
                syslog(LOG_INFO,
                        "[TCP] IPv6 dual-stack request failed listener=%s address=%s port=%d errno=%d",
                        name, BindAddress, port, errno);
            }
            else {
                dual_stack_enabled = 1;
            }
        }

        rc = bind(fd, candidate->ai_addr, candidate->ai_addrlen);
        dbprintf("bind(%s) %d/%d\n", name, rc, errno);
        if (rc < 0) {
            syslog(LEVEL,
                    "[TCP] bind failed listener=%s address=%s port=%d family=%s dual_stack=%s errno=%d",
                    name, BindAddress, port,
                    socket_family_name(candidate->ai_family),
                    dual_stack_failed ? "failed" :
                    (dual_stack_enabled ? "enabled" : "not_applicable"),
                    errno);
            close(fd);
            fd = -1;
            continue;
        }

        rc = listen(fd, 128);
        dbprintf("listen(%s) %d/%d\n", name, rc, errno);
        if (rc < 0) {
            syslog(LEVEL,
                    "[TCP] listen failed listener=%s address=%s port=%d family=%s errno=%d",
                    name, BindAddress, port,
                    socket_family_name(candidate->ai_family), errno);
            close(fd);
            fd = -1;
            continue;
        }

        if (candidate->ai_family == AF_INET6) {
            dual_stack = dual_stack_failed ? "failed" :
                    (dual_stack_enabled ? "enabled" : "unknown");
        }
        ioctl(fd, FIONBIO, &on);
        syslog(LOG_NOTICE,
                "[TCP] listener ready name=%s address=%s port=%d family=%s dual_stack=%s",
                name, BindAddress, port,
                socket_family_name(candidate->ai_family), dual_stack);
        break;
    }

    freeaddrinfo(results);
    if (fd < 0) {
        syslog(LEVEL, "[TCP] could not start listener=%s address=%s port=%d",
                name, BindAddress, port);
    }
    return fd;
}

static void close_listener(const char *name, int *fd)
{
    if (*fd < 0)
        return;

    syslog(LOG_NOTICE, "[TCP] closing listener name=%s fd=%d", name, *fd);
    close(*fd);
    *fd = -1;
}

static int mydaemon(void)
{
    int nready, i;

    /**** sockets ****/
    socklen_t clilen; 
    int clifd;
    int listenfd = -1;
    int flashxmlfd = -1;
    int or20fd = -1;
    struct sockaddr_storage cliaddr;
    unsigned char buf[1024];
    int bytesIn;

//   struct sockaddr_in cliaddr, servaddr;

    /**** USB ****/
    struct sigaction sigact;
    int r = 1;
    const struct libusb_pollfd **usbfds;
    nfds_t nusbfds;
    struct timeval timeout;

    hua_sec_init();

    syslog(LOG_NOTICE, "[USB] initializing libusb");
    r = libusb_init(NULL);
    if (r < 0) {
        syslog(LEVEL,
                "[USB] libusb initialization failed rc=%d; check USB permissions and container passthrough",
                r);
        dbprintf("failed to initialise libusb %d\n", r);
        return 1;
    }
    syslog(LOG_NOTICE, "[USB] libusb initialized");
    libusb_set_debug(NULL, 3);

#if 0
    /* This function is not available in older versions of libusb-1.0 */
    r = libusb_pollfds_handle_timeouts(NULL);
    if (!r) {
        dbprintf("poll timeout required %d\n", r);
        goto out;
    }
#endif
    syslog(LOG_NOTICE, "[USB] looking for CM15A/CM19A controller");
    r = find_cm15a(&Devh);
    if (r < 0) {
        syslog(LEVEL,
                "[USB] could not open CM15A/CM19A rc=%d; check USB passthrough, permissions, and kernel drivers such as ati_remote",
                r);
        dbprintf("Could not find/open CM15A/CM19A %d\n", r);
        goto out;
    }

    r = get_endpoint_address(Devh, &InEndpoint, &OutEndpoint);
    if (r < 0) {
        syslog(LEVEL,
                "[USB] could not find interrupt endpoints rc=%d; unsupported or unavailable controller descriptor",
                r);
        dbprintf("Could not find endpoints %d\n", r);
        goto out_deinit;
    }
    syslog(LOG_NOTICE, "[USB] endpoints ready in=0x%02X out=0x%02X",
            InEndpoint, OutEndpoint);

    r = do_init();
    if (r < 0)
        goto out_deinit;
    syslog(LOG_NOTICE, "[USB] controller initialized");

    r = alloc_transfers();
    if (r < 0)
        goto out_deinit;

    r = start_transfers();
    if (r < 0)
        goto out_deinit;
    syslog(LOG_NOTICE, "[USB] transfers started");

    sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;

    sigaction(SIGINT,  &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    syslog(LOG_NOTICE,
            "[STARTUP] signal handlers installed signals=SIGINT,SIGTERM,SIGQUIT");

    usbfds = libusb_get_pollfds(NULL);
    if (!usbfds) {
        syslog(LEVEL, "[USB] libusb poll descriptor lookup failed");
        r = 1;
        goto out_deinit;
    }
    dbprintf("usbfds %p %p %p %p %p\n", usbfds, 
            usbfds[0], usbfds[1], usbfds[2], usbfds[3]);
    nusbfds = 3;        /* Skip over listen fd at [0,1,2] */
    for (i = 0; usbfds[i] != NULL; i++) {
        dbprintf(" %lu: %p fd %d %04X\n", nusbfds, 
                usbfds[i], usbfds[i]->fd, usbfds[i]->events);
        Clients[nusbfds].fd = usbfds[i]->fd;
        Clients[nusbfds].events = usbfds[i]->events;
        Clients[nusbfds].revents = 0;
        nusbfds++;
    }
    nusbfds -= 3;  /* Adjust for skipping 0,1,2 */
    dbprintf("nusbfds %lu\n", nusbfds);
    syslog(LOG_NOTICE, "[USB] poll descriptors ready count=%lu",
            (unsigned long)nusbfds);
    memset(&timeout, 0, sizeof(timeout));

    if (Cm19a)
        initcm1Xa(initcm19abinary);
    else
        initcm1Xa(initcm15abinary);

    /**** sockets ****/
    listenfd = create_listener("main", ServerPort);
    if (XmlEnabled) {
        flashxmlfd = create_listener("xml", XmlPort);
    }
    else {
        syslog(LOG_NOTICE, "[TCP] optional service disabled name=xml port=%d",
                XmlPort);
    }
    if (OpenRemoteEnabled) {
        or20fd = create_listener("openremote", OpenRemotePort);
    }
    else {
        syslog(LOG_NOTICE,
                "[TCP] optional service disabled name=openremote port=%d",
                OpenRemotePort);
    }
    if (listenfd < 0 || (XmlEnabled && flashxmlfd < 0) ||
            (OpenRemoteEnabled && or20fd < 0)) {
        syslog(LEVEL,
                "[TCP] listener startup failed; closing any listeners that were already opened");
        goto out_deinit;
    }

    init_client();

    Clients[0].fd = listenfd;
    Clients[0].events = POLLIN;

    Clients[1].fd = flashxmlfd;
    Clients[1].events = flashxmlfd >= 0 ? POLLIN : 0;

    Clients[2].fd = or20fd;
    Clients[2].events = or20fd >= 0 ? POLLIN : 0;

    PollTimeOut = -1;
    syslog(LOG_NOTICE,
            "[TCP] services configured address=%s main=enabled:%d xml=%s:%d openremote=%s:%d",
            BindAddress, ServerPort, XmlEnabled ? "enabled" : "disabled",
            XmlPort, OpenRemoteEnabled ? "enabled" : "disabled",
            OpenRemotePort);
    syslog(LOG_NOTICE, "[STARTUP] mochad is running");

    while (!Do_exit) {
        int nsockclients;
        int npollfds;

        /* Start appending records for socket clients to Clients array after 
         * listen, flashxml listen, or20 listen, and USB records
         */
        nsockclients = copy_clients(&Clients[3+nusbfds]);
        /* 1 for listen socket, 1 for flashxml listen socket, 1 for or20 listen
         * socket, nusbfds for libusb, nsockclients for socket clients
        */
        npollfds = 3 + nusbfds + nsockclients;
        nready = poll(Clients, npollfds, PollTimeOut);
        if (nready < 0) {
            if (errno == EINTR) {
                syslog(LOG_DEBUG, "[SHUTDOWN] poll interrupted by signal");
                continue;
            }
            syslog(LEVEL, "[TCP] poll failed errno=%d; shutting down", errno);
            Do_exit = 2;
            break;
        }
#if 0
        dbprintf("poll() %d\n", nready);
        for (i = 0; i < npollfds; i++) {
            dbprintf("Clients[%d] fd %d events %X revents %X\n",
                    i, Clients[i].fd, Clients[i].events, Clients[i].revents);
        }
#endif
        /**** Time out ****/
        if (nready == 0) {
            send_next_x10out();
        }
        else {
            /**** USB ****/
            libusb_handle_events_timeout(NULL, &timeout);

            /**** listen sockets ****/
            if (Clients[0].revents & POLLIN) {
                /* new client connection */
                clilen = sizeof(cliaddr);
                clifd  = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
                log_accept_result("client", clifd);
                if (clifd >= 0) {
                    r = add_client(clifd);
                    if (r < 0) close(clifd);
                }
                if (--nready <= 0) continue;
            }
            if (flashxmlfd >= 0 && (Clients[1].revents & POLLIN)) {
                /* new flashxml client connection */
                clilen = sizeof(cliaddr);
                clifd  = accept(flashxmlfd, (struct sockaddr *)&cliaddr, &clilen);
                log_accept_result("flashxml", clifd);
                if (clifd >= 0) {
                    r = add_xmlclient(clifd);
                    if (r < 0) close(clifd);
                }
                if (--nready <= 0) continue;
            }

            if (or20fd >= 0 && (Clients[2].revents & POLLIN)) {
                /* new OR2.0 client connection */
                clilen = sizeof(cliaddr);
                clifd  = accept(or20fd, (struct sockaddr *)&cliaddr, &clilen);
                log_accept_result("or20", clifd);
                if (clifd >= 0) {
                    r = add_or20client(clifd);
                    if (r < 0) close(clifd);
                }
                if (--nready <= 0) continue;
            }

            for (i = 3+nusbfds; i < npollfds; i++) {
                if ((clifd = Clients[i].fd) >= 0) {
                    /* dbprintf("client %d revents 0x%X\n", i, Clients[i].revents); */
                    if (Clients[i].revents & (POLLIN|POLLERR)) {
                        if ((bytesIn = read(clifd, buf, sizeof(buf))) < 0) {
                            dbprintf("read err %d\n", errno);
                            if (errno != ECONNRESET) {
                                dbprintf("serious error %d\n", errno);
                            }
                            del_client(clifd);
                        }
                        else if (bytesIn == 0) {
                            dbprintf("read EOF %d\n", bytesIn);
                            del_client(clifd);
                        }
                        else {
                            cm15a_encode_state_t *state;
			    dbprintf("Input: %.*s", bytesIn, (char *)buf);
                            syslog(LOG_DEBUG,
                                    "[COMMAND] received client_id=%u fd=%d bytes=%d",
                                    client_id_for_fd(clifd), clifd, bytesIn);
                            state = client_encode_state(clifd);
                            if (state)
                                cm15a_encode_with_state(clifd, state, buf, (size_t)bytesIn);
                        }
                        if (--nready <= 0) break;
                    }
                }
            }
        }
    }
    syslog(LOG_NOTICE, "[SHUTDOWN] detaching controller model=%s",
            (Cm19a) ? "CM19A" : "CM15A");

    if (IntrOut_transfer) {
        r = libusb_cancel_transfer(IntrOut_transfer);
        if (r < 0) {
            syslog(LEVEL, "[SHUTDOWN] interrupt output transfer cancel failed rc=%d",
                    r);
            goto out_deinit;
        }
    }

    if (IntrIn_transfer) {
        r = libusb_cancel_transfer(IntrIn_transfer);
        if (r < 0) {
            syslog(LEVEL, "[SHUTDOWN] interrupt input transfer cancel failed rc=%d",
                    r);
            goto out_deinit;
        }
    }

    i = 100;
    while ((IntrOut_transfer || IntrIn_transfer) && i--)
        if (libusb_handle_events(NULL) < 0)
            break;

    if (Do_exit == 1) {
        syslog(LOG_NOTICE, "[SHUTDOWN] requested by %s (%d)",
                signal_name(Exit_signal), Exit_signal);
        r = 0;
    }
    else {
        syslog(LOG_NOTICE, "[SHUTDOWN] stopping after USB or poll error");
        r = 1;
    }

out_deinit:
    close_listener("main", &listenfd);
    close_listener("xml", &flashxmlfd);
    close_listener("openremote", &or20fd);
    syslog(LOG_NOTICE, "[SHUTDOWN] releasing USB resources");
    libusb_free_transfer(IntrIn_transfer);
    libusb_free_transfer(IntrOut_transfer);
/* out_release: */
    if (Devh) {
        r = libusb_release_interface(Devh, 0);
        if (r < 0) {
            syslog(LEVEL, "[SHUTDOWN] release interface failed rc=%d", r);
        }
        if (Reattach) {
            r = libusb_attach_kernel_driver(Devh, 0);
            if (r < 0) {
                syslog(LEVEL, "[SHUTDOWN] kernel driver reattach failed rc=%d",
                        r);
            }
            else {
                syslog(LOG_NOTICE, "[SHUTDOWN] kernel driver reattached");
            }
        }
    }
out:
    if (Devh)
        libusb_close(Devh);
    libusb_exit(NULL);
    syslog(LOG_NOTICE, "[SHUTDOWN] complete");
    return r >= 0 ? r : -r;
}

static void printcopy(void)
{
    printf("Copyright (C) 2010-2012 Brian Uechi.\n");
    printf("\n");
    printf("This program comes with NO WARRANTY.\n");
    printf("You may redistribute copies of this program\n");
    printf("under the terms of the GNU General Public License.\n");
    printf("For more information about these matters, see the file named LICENSE.md.\n");
    fflush(NULL);
}

void 
help() {
    printf("Copyright (C) 2010-2014 Brian Uechi.\n");
    printf("Copyright (C) 2014 Neil Cherry.\n");
    printf("    -d - run in foreground\n");
    printf("    --bind ADDRESS - bind TCP listeners to IPv4 or IPv6 address (default 0.0.0.0)\n");
    printf("    --port PORT - main TCP port (default 1099)\n");
    printf("    --enable-xml - enable Flash XMLSocket listener (default)\n");
    printf("    --disable-xml - disable Flash XMLSocket listener\n");
    printf("    --xml-port PORT - Flash XMLSocket port (default 1100)\n");
    printf("    --enable-openremote - enable OpenRemote 2.0 listener (default)\n");
    printf("    --disable-openremote - disable OpenRemote 2.0 listener\n");
    printf("    --openremote-port PORT - OpenRemote 2.0 port (default 1101)\n");
    printf("    --raw-data\n");
    printf("    --raw-date\n");
    printf("    --version\n");
    printf("    --help\n");
    printf("\n");
}

// This affects whether decode.c will show raw frame data for debugging RF connectivity
// as well as providing raw data for parsing by users like misterhouse's X10_CMxx module.
int raw_data = 0;
int main(int argc, char *argv[])
{
    int rc, i;
    int foreground=0;

    /* Process command line args */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0)
            foreground = 1;
        else if (strcmp(argv[i], "--raw-data") == 0)
            raw_data = 1;
        else if (strcmp(argv[i], "--bind") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "--bind requires an IPv4 or IPv6 address\n");
                exit(-1);
            }
            BindAddress = argv[i];
        }
        else if (strcmp(argv[i], "--port") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "--port requires a TCP port\n");
                exit(-1);
            }
            if (parse_port_option("--port", argv[i], &ServerPort) < 0)
                exit(-1);
        }
        else if (strcmp(argv[i], "--enable-xml") == 0) {
            XmlEnabled = 1;
        }
        else if (strcmp(argv[i], "--disable-xml") == 0) {
            XmlEnabled = 0;
        }
        else if (strcmp(argv[i], "--xml-port") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "--xml-port requires a TCP port\n");
                exit(-1);
            }
            if (parse_port_option("--xml-port", argv[i], &XmlPort) < 0)
                exit(-1);
        }
        else if (strcmp(argv[i], "--enable-openremote") == 0) {
            OpenRemoteEnabled = 1;
        }
        else if (strcmp(argv[i], "--disable-openremote") == 0) {
            OpenRemoteEnabled = 0;
        }
        else if (strcmp(argv[i], "--openremote-port") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "--openremote-port requires a TCP port\n");
                exit(-1);
            }
            if (parse_port_option("--openremote-port", argv[i], &OpenRemotePort) < 0)
                exit(-1);
        }
        else if (strcmp(argv[i], "--version") == 0) {
            printf("%s\n", REDUX_VERSION);
            printf("upstream base: %s\n", PACKAGE_STRING);
            printcopy();
            exit(0);
        } else if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            printf("%s\n", REDUX_VERSION);
            printf("upstream base: %s\n", PACKAGE_STRING);
	    help();
            exit(0);
        }
        else {
            printf("unknown option %s\n", argv[i]);
            exit(-1);
        }
    }
    if (validate_runtime_config() < 0)
        exit(-1);

    /* Initialize logging after argument parsing so foreground mode can mirror
     * friendly lifecycle messages to stderr for containers and manual tests.
     */
    StartTime = time(NULL);
    openlog(DAEMON_NAME, LOG_PID | (foreground ? LOG_PERROR : 0), LOG_LOCAL5);
    syslog(LOG_NOTICE,
            "[STARTUP] %s starting (upstream_base=\"%s\", foreground=%s, raw_data=%s)",
            REDUX_VERSION, PACKAGE_STRING, foreground ? "yes" : "no",
            raw_data ? "yes" : "no");
    syslog(LOG_NOTICE,
            "[STARTUP] TCP configuration bind=%s main=enabled:%d xml=%s:%d openremote=%s:%d",
            BindAddress, ServerPort, XmlEnabled ? "enabled" : "disabled",
            XmlPort, OpenRemoteEnabled ? "enabled" : "disabled",
            OpenRemotePort);

    /* Daemonize */
    if (!foreground) {
        rc = daemon(0, 0);
        dbprintf("daemon() => %d\n", rc);
        syslog(LOG_NOTICE, "[STARTUP] running in background");
    }
    else {
        syslog(LOG_NOTICE, "[STARTUP] running in foreground");
    }

    /* Do real work */
    rc = mydaemon();

    /* Finish up */
    syslog(LOG_NOTICE, "[SHUTDOWN] terminated");
    closelog();
    return rc;
}

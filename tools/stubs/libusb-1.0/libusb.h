/*
 * Development-only libusb header stub.
 *
 * This file exists so maintainers can syntax-check mochad.c in environments
 * that do not have libusb development headers installed. It is intentionally
 * incomplete and must never be used for a production build or runtime test.
 */

#ifndef MOCHAD_REDUX_LIBUSB_STUB_H
#define MOCHAD_REDUX_LIBUSB_STUB_H

#include <stdint.h>
#include <sys/time.h>

#define LIBUSB_API_VERSION 0x01000102

#define LIBUSB_SUCCESS 0
#define LIBUSB_ERROR_IO -1
#define LIBUSB_ERROR_INVALID_PARAM -2
#define LIBUSB_ERROR_ACCESS -3
#define LIBUSB_ERROR_NO_DEVICE -4
#define LIBUSB_ERROR_NOT_FOUND -5
#define LIBUSB_ERROR_BUSY -6
#define LIBUSB_ERROR_TIMEOUT -7
#define LIBUSB_ERROR_OVERFLOW -8
#define LIBUSB_ERROR_PIPE -9
#define LIBUSB_ERROR_INTERRUPTED -10
#define LIBUSB_ERROR_NO_MEM -11
#define LIBUSB_ERROR_NOT_SUPPORTED -12
#define LIBUSB_ERROR_OTHER -99

#define LIBUSB_TRANSFER_COMPLETED 0
#define LIBUSB_TRANSFER_ERROR 1
#define LIBUSB_TRANSFER_TIMED_OUT 2
#define LIBUSB_TRANSFER_CANCELLED 3
#define LIBUSB_TRANSFER_STALL 4
#define LIBUSB_TRANSFER_NO_DEVICE 5
#define LIBUSB_TRANSFER_OVERFLOW 6

#define LIBUSB_ENDPOINT_IN 0x80
#define LIBUSB_TRANSFER_TYPE_MASK 0x03
#define LIBUSB_TRANSFER_TYPE_BULK 0x02
#define LIBUSB_TRANSFER_TYPE_INTERRUPT 0x03

#define LIBUSB_CAP_HAS_HOTPLUG 0x00000001
#define LIBUSB_HOTPLUG_MATCH_ANY -1
#define LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED 0x01
#define LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT 0x02

typedef struct libusb_device_handle libusb_device_handle;
typedef struct libusb_device libusb_device;
typedef struct libusb_context libusb_context;
typedef int libusb_hotplug_event;
typedef int libusb_hotplug_callback_handle;

struct libusb_device_descriptor {
    uint16_t idVendor;
    uint16_t idProduct;
};

struct libusb_endpoint_descriptor {
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
};

struct libusb_interface_descriptor {
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    const struct libusb_endpoint_descriptor *endpoint;
};

struct libusb_interface {
    const struct libusb_interface_descriptor *altsetting;
    int num_altsetting;
};

struct libusb_config_descriptor {
    uint8_t bNumInterfaces;
    const struct libusb_interface *interface;
};

struct libusb_transfer {
    int status;
    int actual_length;
    unsigned char *buffer;
};

struct libusb_pollfd {
    int fd;
    short events;
};

typedef void (*libusb_transfer_cb_fn)(struct libusb_transfer *transfer);
typedef void (*libusb_pollfd_added_cb)(int fd, short events, void *user_data);
typedef void (*libusb_pollfd_removed_cb)(int fd, void *user_data);
typedef int (*libusb_hotplug_callback_fn)(libusb_context *ctx,
        libusb_device *device, libusb_hotplug_event event, void *user_data);

libusb_device_handle *libusb_open_device_with_vid_pid(libusb_context *ctx,
        uint16_t vendor_id, uint16_t product_id);
int libusb_claim_interface(libusb_device_handle *devh, int interface_number);
int libusb_set_auto_detach_kernel_driver(libusb_device_handle *devh,
        int enable);
int libusb_kernel_driver_active(libusb_device_handle *devh,
        int interface_number);
int libusb_detach_kernel_driver(libusb_device_handle *devh,
        int interface_number);
libusb_device *libusb_get_device(libusb_device_handle *devh);
int libusb_get_device_descriptor(libusb_device *dev,
        struct libusb_device_descriptor *desc);
int libusb_get_active_config_descriptor(libusb_device *dev,
        struct libusb_config_descriptor **config);
void libusb_free_config_descriptor(struct libusb_config_descriptor *config);
struct libusb_transfer *libusb_alloc_transfer(int iso_packets);
void libusb_free_transfer(struct libusb_transfer *transfer);
void libusb_fill_interrupt_transfer(struct libusb_transfer *transfer,
        libusb_device_handle *devh, unsigned char endpoint,
        unsigned char *buffer, int length, libusb_transfer_cb_fn callback,
        void *user_data, unsigned int timeout);
int libusb_submit_transfer(struct libusb_transfer *transfer);
int libusb_cancel_transfer(struct libusb_transfer *transfer);
const char *libusb_error_name(int error_code);
int libusb_has_capability(uint32_t capability);
int libusb_hotplug_register_callback(libusb_context *ctx, int events,
        int flags, int vendor_id, int product_id, int dev_class,
        libusb_hotplug_callback_fn callback_fn, void *user_data,
        libusb_hotplug_callback_handle *callback_handle);
void libusb_hotplug_deregister_callback(libusb_context *ctx,
        libusb_hotplug_callback_handle callback_handle);
int libusb_handle_events(libusb_context *ctx);
int libusb_init(libusb_context **ctx);
void libusb_set_debug(libusb_context *ctx, int level);
int libusb_pollfds_handle_timeouts(libusb_context *ctx);
const struct libusb_pollfd **libusb_get_pollfds(libusb_context *ctx);
void libusb_set_pollfd_notifiers(libusb_context *ctx,
        libusb_pollfd_added_cb added_cb, libusb_pollfd_removed_cb removed_cb,
        void *user_data);
int libusb_get_next_timeout(libusb_context *ctx, struct timeval *tv);
int libusb_handle_events_timeout(libusb_context *ctx, struct timeval *tv);
int libusb_release_interface(libusb_device_handle *devh,
        int interface_number);
int libusb_attach_kernel_driver(libusb_device_handle *devh,
        int interface_number);
void libusb_close(libusb_device_handle *devh);
void libusb_exit(libusb_context *ctx);

#endif

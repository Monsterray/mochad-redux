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

#define LIBUSB_TRANSFER_COMPLETED 0

typedef struct libusb_device_handle libusb_device_handle;
typedef struct libusb_device libusb_device;

struct libusb_device_descriptor {
    uint16_t idProduct;
};

struct libusb_endpoint_descriptor {
    uint8_t bEndpointAddress;
};

struct libusb_interface_descriptor {
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

libusb_device_handle *libusb_open_device_with_vid_pid(void *ctx,
        uint16_t vendor_id, uint16_t product_id);
int libusb_claim_interface(libusb_device_handle *devh, int interface_number);
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
int libusb_handle_events(void *ctx);
int libusb_init(void *ctx);
void libusb_set_debug(void *ctx, int level);
int libusb_pollfds_handle_timeouts(void *ctx);
const struct libusb_pollfd **libusb_get_pollfds(void *ctx);
int libusb_handle_events_timeout(void *ctx, struct timeval *tv);
int libusb_release_interface(libusb_device_handle *devh,
        int interface_number);
int libusb_attach_kernel_driver(libusb_device_handle *devh,
        int interface_number);
void libusb_close(libusb_device_handle *devh);
void libusb_exit(void *ctx);

#endif

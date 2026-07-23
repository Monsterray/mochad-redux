#include "usb_endpoint_selection.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

static int run_case(const char *name, const struct libusb_config_descriptor *config, int expected) {
    uint8_t in_endpoint = 0;
    uint8_t out_endpoint = 0;
    unsigned int in_packet = 0;
    unsigned int out_packet = 0;
    int r;

    r = mochad_select_interrupt_endpoints(config, 0, 0, 8, 8, &in_endpoint, &out_endpoint,
                                          &in_packet, &out_packet);
    if (r != expected) {
        fprintf(stderr, "FAIL: %s returned %d expected %d\n", name, r, expected);
        return 1;
    }
    if (expected == 0) {
        if (expect(in_endpoint == 0x81, "wrong IN endpoint"))
            return 1;
        if (expect(out_endpoint == 0x02, "wrong OUT endpoint"))
            return 1;
        if (expect(in_packet == 8, "wrong IN packet size"))
            return 1;
        if (expect(out_packet == 8, "wrong OUT packet size"))
            return 1;
    }
    return 0;
}

int main(void) {
    struct libusb_endpoint_descriptor valid_eps[] = {
        {.bEndpointAddress = 0x81,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
        {.bEndpointAddress = 0x02,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
    };
    struct libusb_interface_descriptor valid_alt[] = {
        {.bInterfaceNumber = 0, .bAlternateSetting = 0, .bNumEndpoints = 2, .endpoint = valid_eps},
    };
    struct libusb_interface valid_iface[] = {
        {.altsetting = valid_alt, .num_altsetting = 1},
    };
    struct libusb_config_descriptor valid_config = {
        .bNumInterfaces = 1,
        .interface = valid_iface,
    };

    struct libusb_endpoint_descriptor wrong_type_eps[] = {
        {.bEndpointAddress = 0x81, .bmAttributes = LIBUSB_TRANSFER_TYPE_BULK, .wMaxPacketSize = 8},
        {.bEndpointAddress = 0x02,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
    };
    struct libusb_interface_descriptor wrong_type_alt[] = {
        {.bInterfaceNumber = 0,
         .bAlternateSetting = 0,
         .bNumEndpoints = 2,
         .endpoint = wrong_type_eps},
    };
    struct libusb_interface wrong_type_iface[] = {
        {.altsetting = wrong_type_alt, .num_altsetting = 1},
    };
    struct libusb_config_descriptor wrong_type_config = {
        .bNumInterfaces = 1,
        .interface = wrong_type_iface,
    };

    struct libusb_endpoint_descriptor duplicate_eps[] = {
        {.bEndpointAddress = 0x81,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
        {.bEndpointAddress = 0x82,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
        {.bEndpointAddress = 0x02,
         .bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT,
         .wMaxPacketSize = 8},
    };
    struct libusb_interface_descriptor duplicate_alt[] = {
        {.bInterfaceNumber = 0,
         .bAlternateSetting = 0,
         .bNumEndpoints = 3,
         .endpoint = duplicate_eps},
    };
    struct libusb_interface duplicate_iface[] = {
        {.altsetting = duplicate_alt, .num_altsetting = 1},
    };
    struct libusb_config_descriptor duplicate_config = {
        .bNumInterfaces = 1,
        .interface = duplicate_iface,
    };

    struct libusb_interface_descriptor wrong_alt[] = {
        {.bInterfaceNumber = 0, .bAlternateSetting = 1, .bNumEndpoints = 2, .endpoint = valid_eps},
    };
    struct libusb_interface wrong_alt_iface[] = {
        {.altsetting = wrong_alt, .num_altsetting = 1},
    };
    struct libusb_config_descriptor wrong_alt_config = {
        .bNumInterfaces = 1,
        .interface = wrong_alt_iface,
    };

    if (run_case("valid", &valid_config, 0))
        return 1;
    if (run_case("wrong transfer type", &wrong_type_config, -ENODEV))
        return 1;
    if (run_case("duplicate candidate", &duplicate_config, -ENODEV))
        return 1;
    if (run_case("wrong alternate setting", &wrong_alt_config, -ENODEV))
        return 1;

    valid_eps[0].wMaxPacketSize = 7;
    if (run_case("insufficient packet size", &valid_config, -ENODEV))
        return 1;

    return 0;
}

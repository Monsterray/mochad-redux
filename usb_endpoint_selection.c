#include "usb_endpoint_selection.h"

#include <errno.h>

int mochad_select_interrupt_endpoints(
        const struct libusb_config_descriptor *config,
        int interface_number,
        int alternate_setting,
        size_t min_in_packet_size,
        size_t min_out_packet_size,
        uint8_t *in_endpoint,
        uint8_t *out_endpoint,
        unsigned int *in_packet_size,
        unsigned int *out_packet_size)
{
    int i;
    int j;
    int found_interface = 0;
    int found_altsetting = 0;
    int in_count = 0;
    int out_count = 0;

    if (in_endpoint == NULL || out_endpoint == NULL ||
            in_packet_size == NULL || out_packet_size == NULL)
        return -EINVAL;

    *in_endpoint = 0;
    *out_endpoint = 0;
    *in_packet_size = 0;
    *out_packet_size = 0;

    if (config == NULL || interface_number < 0 || alternate_setting < 0)
        return -EINVAL;

    for (i = 0; i < config->bNumInterfaces; i++) {
        const struct libusb_interface *iface = &config->interface[i];

        for (j = 0; j < iface->num_altsetting; j++) {
            const struct libusb_interface_descriptor *alt =
                    &iface->altsetting[j];
            int k;

            if (alt->bInterfaceNumber != interface_number)
                continue;
            found_interface = 1;

            if (alt->bAlternateSetting != alternate_setting)
                continue;
            found_altsetting = 1;

            for (k = 0; k < alt->bNumEndpoints; k++) {
                const struct libusb_endpoint_descriptor *endpoint =
                        &alt->endpoint[k];
                unsigned int packet_size =
                        endpoint->wMaxPacketSize & 0x07ff;
                int is_interrupt = (endpoint->bmAttributes &
                        LIBUSB_TRANSFER_TYPE_MASK) ==
                        LIBUSB_TRANSFER_TYPE_INTERRUPT;

                if (!is_interrupt)
                    continue;

                if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                    if (packet_size < min_in_packet_size)
                        continue;
                    *in_endpoint = endpoint->bEndpointAddress;
                    *in_packet_size = packet_size;
                    in_count++;
                }
                else {
                    if (packet_size < min_out_packet_size)
                        continue;
                    *out_endpoint = endpoint->bEndpointAddress;
                    *out_packet_size = packet_size;
                    out_count++;
                }
            }
        }
    }

    if (!found_interface || !found_altsetting)
        return -ENODEV;
    if (in_count != 1 || out_count != 1)
        return -ENODEV;

    return 0;
}

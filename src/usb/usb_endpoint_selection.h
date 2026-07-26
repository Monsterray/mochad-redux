#ifndef USB_ENDPOINT_SELECTION_H
#define USB_ENDPOINT_SELECTION_H

#include <stddef.h>
#include <stdint.h>

#include <libusb-1.0/libusb.h>

int mochad_select_interrupt_endpoints(const struct libusb_config_descriptor *config,
                                      int interface_number, int alternate_setting,
                                      size_t min_in_packet_size, size_t min_out_packet_size,
                                      uint8_t *in_endpoint, uint8_t *out_endpoint,
                                      unsigned int *in_packet_size, unsigned int *out_packet_size);

#endif

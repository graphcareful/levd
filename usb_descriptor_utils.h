#include <libusb-1.0/libusb.h>
#include <string>

#include "constants.h"

bool incoming_endpoint(const libusb_endpoint_descriptor &endpoint);

std::string get_serial_number(libusb_device_descriptor desc,
                              libusb_device_handle *   handle);
libusb_device_descriptor get_descriptor(libusb_device *device);
libusb_device_handle *get_handle(libusb_device *device);
libusb_config_descriptor *get_config_descriptor(libusb_device *device);
libusb_interface_descriptor get_main_usb_interface(
  libusb_config_descriptor *config);

void set_endpoints(const libusb_endpoint_descriptor *endpoints,
                   libusb_endpoint_descriptor &endpointIn,
                   libusb_endpoint_descriptor &endpointOut);

bool transfer_bulk_raw_data(libusb_device_handle *handle,
                            unsigned char         endpoint,
                            unsigned char *       data,
                            size_t                length);

bool transfer_control_value(libusb_device_handle *handle, uint16_t value);

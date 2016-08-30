#include "usb_descriptor_utils.h"
#include <glog/logging.h>

#define kMainConfigurationIndex 0
#define kMainConfigurationValue 1

bool incoming_endpoint(const libusb_endpoint_descriptor &endpoint) {
  // Or alternatively if this bit operation isn't == 0
  return (endpoint.bEndpointAddress & LIBUSB_ENDPOINT_IN) == LIBUSB_ENDPOINT_IN;
}

std::string get_serial_number(libusb_device_descriptor desc,
                              libusb_device_handle *   handle) {
  unsigned char data[256];
  int no_bytes = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
                                                    data, sizeof(data));
  return (no_bytes != 0) ? std::string(reinterpret_cast<char *>(data)) : "";
}

libusb_device_descriptor get_descriptor(libusb_device *device) {
  struct libusb_device_descriptor desc = {0};
  int err = libusb_get_device_descriptor(device, &desc);
  CHECK(err == 0) << "Failed to get descriptor for generic device";
  CHECK(desc.bNumConfigurations == 1)
    << "Should only be one configuration descriptor for the kraken";
  CHECK(desc.idVendor == KRAKEN_X61_VENDOR
        && desc.idProduct == KRAKEN_X61_PRODUCT)
    << "This method expects a valid kraken device as its parameter";
  return desc;
}

libusb_device_handle *get_handle(libusb_device *device) {
  libusb_device_handle *handle = NULL;
  int                   ret    = libusb_open(device, &handle);
  switch (ret) {
  case LIBUSB_ERROR_NO_MEM:
    LOG(FATAL) << "Out of memory";
  case LIBUSB_ERROR_ACCESS:
    LOG(FATAL) << "Insufficient permissions";
  case LIBUSB_ERROR_NO_DEVICE:
    LOG(FATAL) << "Device disconnected";
  }
  if (LIBUSB_SUCCESS != ret || handle == NULL) {
    LOG(FATAL) << "ERROR when calling libusb_open";
  }
  return handle;
}

libusb_config_descriptor *get_config_descriptor(libusb_device *device) {
  libusb_config_descriptor *config = NULL;
  int                       rc =
    libusb_get_config_descriptor(device, kMainConfigurationIndex, &config);
  CHECK(rc == 0 && config != NULL) << "Error when retrieving current device "
                                      "configuration, continuing as normal";
  return config;
}

// TODO: Refactor
libusb_interface_descriptor get_main_usb_interface(
  libusb_config_descriptor *config) {
  // The config descriptor has an array interfaces supported by this
  // configuration
  // size of this array is == bNumInterfaces
  const libusb_interface *interfaces = config->interface;

  // Array of interface descriptors
  // size of this array is == interfaces[0]->num_altsetting
  const libusb_interface_descriptor *interface_descriptors =
    interfaces[0].altsetting;

  // Actual sole interface descriptor for kraken
  // DescriptorType should be 4
  // bNumEndpoints should be == 2
  // Interface class should be 255 (vendor specirfic)
  return interface_descriptors[0];
}

void set_endpoints(const libusb_endpoint_descriptor *endpoints,
                   libusb_endpoint_descriptor &endpointIn,
                   libusb_endpoint_descriptor &endpointOut) {
  if (incoming_endpoint(endpoints[1])) {
    endpointIn = endpoints[1];
  } else {
    endpointOut = endpoints[1];
  }
  if (incoming_endpoint(endpoints[0])) {
    endpointIn = endpoints[0];
  } else {
    endpointOut = endpoints[0];
  }
}

bool transfer_bulk_raw_data(libusb_device_handle *handle,
                            unsigned char         endpoint,
                            unsigned char *       data,
                            size_t                length) {
  static const unsigned int kKrakenUsbTimeout = 1000;
  unsigned char *           head              = data;
  int *                     transferred       = new int;
  size_t                    bytes_sent        = 0;
  while (bytes_sent < length) {
    size_t bytes_to_send = std::min((size_t)64, length - bytes_sent);
    int    ret = libusb_bulk_transfer(handle, endpoint, head, bytes_to_send,
                                   transferred, kKrakenUsbTimeout);
    if (ret == 0) {
      VLOG(2) << "Success sending packet, " << *transferred << " bytes sent";
      bytes_sent += bytes_to_send;
      head = data + bytes_sent;
    } else {
      LOG(ERROR) << "Failed to send packed to device";
      break;
    }
  }
  delete[] transferred;
  CHECK(bytes_sent <= length) << "Sent more bytes then should have";
  return (bytes_sent == length) ? true : false;
}

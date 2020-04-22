#pragma once
#include <libusb-1.0/libusb.h>

namespace levd {
libusb_device *leviathan_init(libusb_device **devices, ssize_t num_devices);
void           leviathan_start(libusb_device *kraken_device);
}  // namespace levd

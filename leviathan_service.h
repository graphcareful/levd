#ifndef LEVIATHAN_SERVICE_H
#define LEVIATHAN_SERVICE_H

#include <libusb-1.0/libusb.h>

libusb_device *leviathan_init(libusb_device **devices, ssize_t num_devices);
void leviathan_start(libusb_device *kraken_device);

#endif  // LEVIATHAN_SERVICE_H

#include "build/version.h"
#include "leviathan_service.hpp"
#include <glog/logging.h>
#include <libusb-1.0/libusb.h>

int main(int argc, char *argv[]) {
  FLAGS_logtostderr = 1;
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Starting Levd Daemon version: " << LEVD_VERSION_MAJOR << "."
            << LEVD_VERSION_MINOR;

  const int rc = libusb_init(NULL);
  CHECK(rc == 0) << "Error initializing libusb: " << libusb_error_name(rc);

  libusb_device **devices;
  ssize_t         num_devices = libusb_get_device_list(NULL, &devices);
  if (num_devices == 0) {
    LOG(WARNING) << "There are no usb devices attached";
    return 0;
  }
  LOG(INFO) << "libusb successfully initialized...";
  LOG(INFO) << "There are " << num_devices << " usb devices hooked up";
  libusb_device *kraken_device = leviathan_init(devices, num_devices);
  if (kraken_device) {
    LOG(INFO) << "Kraken X61 is detected";
    LOG(INFO) << "Starting levd service...";
    leviathan_start(kraken_device);
  } else {
    LOG(ERROR) << "Kraken X61 was not detected";
  }

  LOG(INFO) << "... driver gracefully shutting down";
  libusb_free_device_list(devices, true);
  libusb_exit(NULL);
  return 0;
}

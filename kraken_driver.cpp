#include "kraken_driver.h"
#include "usb_descriptor_utils.h"

#include <algorithm>
#include <glog/logging.h>
#include <string.h>

#define kMainConfigurationIndex 0
#define kMainConfigurationValue 1

void set_color_arr(const uint32_t c, unsigned char *arr) {
  arr[0] = (c & 0x00FF0000) >> 16;
  arr[1] = (c & 0x0000FF00) >> 8;
  arr[2] = c & 0x000000FF;
}

// TODO: Make params const
KrakenDriver::KrakenDriver(libusb_device *kraken_device)
  : _device(kraken_device)
  , _desc(get_descriptor(_device))
  , _config(get_config_descriptor(_device))
  , _handle(get_handle(_device)) {
  // Init _color to kDefaultColor, most bytes will never change
  memcpy(_color, kDefaultColor, 19);

  // Grab endpoints via libusb
  CHECK (_config->bConfigurationValue == kMainConfigurationValue)
    << "bConfigurationValue must equal: " << kMainConfigurationValue;
  const int set_configuration_result =
    libusb_set_configuration(_handle, kMainConfigurationValue);
  CHECK(set_configuration_result == 0)
    << "Error when setting kraken usb configuration, got: "
    << libusb_error_name(set_configuration_result);
  const libusb_interface_descriptor main_interface =
    get_main_usb_interface(_config);
  const libusb_endpoint_descriptor *endpoints = main_interface.endpoint;
  CHECK(main_interface.bNumEndpoints == 2) << "Expecting only 2 endpoints";
  set_endpoints(endpoints, _endpointIn, _endpointOut);

  // Send initialization control message, at startup and never again
  sendControlTransfer(KRAKEN_INIT);
}

KrakenDriver::~KrakenDriver() {
  if (_config) {
    libusb_free_config_descriptor(_config);
  }
  if (_handle) {
    libusb_close(_handle);
  }
}

/** ********** Public interface ********** */

std::string KrakenDriver::getSerialNumber() const {
  CHECK(_desc.iSerialNumber)
    << "Expecting Kraken to have string descriptor for device serial number";
  return get_serial_number(_desc, _handle);
}

void KrakenDriver::setFanSpeed(unsigned char fan_speed) {
  CHECK(fan_speed <= 100 && fan_speed >= 30 && fan_speed % 5 == 0)
    << "Fan speed must be between 30 and 100 and divisible by 5: "
    << (uint32_t)fan_speed;
  _fan_speed[1] = fan_speed;
}

void KrakenDriver::setPumpSpeed(unsigned char pump_speed) {
  CHECK(pump_speed <= 100 && pump_speed >= 30 && pump_speed % 5 == 0)
    << "Pump speed must be between 30 and 100 and divisible by 5: "
    << (uint32_t)pump_speed;
  _pump_speed[1] = pump_speed;
}

void KrakenDriver::setColor(uint32_t c) {
  unsigned char color[3];
  set_color_arr(c, color);
  _color[1] = color[0];
  _color[2] = color[1];
  _color[3] = color[2];
}

std::map<std::string, uint32_t> KrakenDriver::sendColorUpdate() {
  sendControlTransfer(KRAKEN_BEGIN);
  sendBulkRawData(_color, 19);
  return receiveStatus();
}

std::map<std::string, uint32_t> KrakenDriver::sendSpeedUpdate() {
  sendControlTransfer(KRAKEN_BEGIN);
  sendBulkRawData(_pump_speed, 2);
  sendBulkRawData(_fan_speed, 2);
  return receiveStatus();
}

/** ********** Private interface ********** */

bool KrakenDriver::sendControlTransfer(uint16_t wValue) {
  return transfer_control_value(_handle, wValue);
}

bool KrakenDriver::sendBulkRawData(unsigned char *data, const size_t length) {
  return transfer_bulk_raw_data(_handle, _endpointOut.bEndpointAddress, data,
                                length);
}

bool KrakenDriver::readBulkRawData(unsigned char *results,
                                   const size_t   length) {
  return transfer_bulk_raw_data(_handle, _endpointIn.bEndpointAddress, results,
                                length);
}

std::map<std::string, uint32_t> KrakenDriver::receiveStatus() {
  unsigned char status[32];
  std::map<std::string, uint32_t> results;
  if (readBulkRawData(status, 32) == false) {
    return results;
  }
  results["fan_speed"]          = 256 * status[0] + status[1];
  results["pump_speed"]         = 256 * status[8] + status[9];
  results["liquid_temperature"] = status[10];
  return results;
}

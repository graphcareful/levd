#pragma once
#include <libusb-1.0/libusb.h>
#include <map>
#include <string>

#include "constants.h"

namespace levd {
// Using this class will query usb bus for the kraken device details
// and set the proper usb configuration to kMainConfigurationIndex
class KrakenDriver {
 public:
  // TODO: Maybe this makes more sense to be a singleton?
  // Creating an instance of this object claims ownership of the usb
  // endpoint, new instances fail to initalize on construction
  KrakenDriver(libusb_device *kraken);
  KrakenDriver(const KrakenDriver &)  = delete;
  KrakenDriver(const KrakenDriver &&) = delete;
  virtual ~KrakenDriver();

  void                            setFanSpeed(unsigned char);
  void                            setPumpSpeed(unsigned char);
  void                            setColor(uint32_t);
  std::map<std::string, uint32_t> sendColorUpdate();
  std::map<std::string, uint32_t> sendSpeedUpdate();

  std::string getSerialNumber() const;

 private:
  bool sendControlTransfer(uint16_t wValue);
  bool sendBulkRawData(unsigned char *data, const size_t length);
  bool readBulkRawData(unsigned char *results, const size_t length);

  std::map<std::string, uint32_t> receiveStatus();

  unsigned char _color[19];
  unsigned char _fan_speed[2]{KRAKEN_FAN_CODE, 30};
  unsigned char _pump_speed[2]{KRAKEN_PUMP_CODE, 30};

 private:
  libusb_device *const            _device;  // Unowned
  const libusb_device_descriptor  _desc;
  libusb_config_descriptor *const _config;
  libusb_device_handle *const     _handle;

  // TODO: Make const
  libusb_endpoint_descriptor _endpointOut;
  libusb_endpoint_descriptor _endpointIn;
};
}  // namespace levd

#pragma once
#include <cstring>
#include <glog/logging.h>
#include <limits>
#include <sensors/sensors.h>
#include <string>
#include <vector>

class CpuTemperatureMonitor {
 public:
  // TODO: Disable copy, permit move semantics
  CpuTemperatureMonitor()
    : cn_(get_chip_name()), subfeatures_(buildSubfeatures()) {}
  CpuTemperatureMonitor(const CpuTemperatureMonitor &) = delete;
  ~CpuTemperatureMonitor() { sensors_cleanup(); }

  // Returns the temperature of the CPU heat sink itself
  uint32_t getPackageIdTemperature() const { return temperatureForCoreId(0); }
  // Returns the temperature of an individual core
  uint32_t getCoreIdTemperature(uint8_t core_id) const {
    // On bad input, normalize instead of returning -1
    if (core_id > coreCount()) {
      core_id = coreCount();
    } else if (core_id < 0) {
      core_id = 0;  // 0th core, package_id is 0
    }
    return temperatureForCoreId(core_id + 1);
  }
  // Returns the number of physical cores in the CPU
  size_t coreCount() const { return subfeatures_.size() - 1; }

 private:
  uint32_t temperatureForCoreId(uint8_t id) const {
    const sensors_subfeature *const subfeature = subfeatures_[id];
    double                          value;
    if (sensors_get_value(cn_, subfeature->number, &value) != 0) {
      LOG(WARNING) << "Failure on call to lmsensors::sensors_get_value()";
      return std::numeric_limits<int>::min();
    }
    return value;
  }

  // Save all pointers to core subfeatures relating to temperature_input on
  // class construction, refer to them later when user queries for core temp
  const std::vector<const sensors_subfeature *> buildSubfeatures() const {
    std::vector<const sensors_subfeature *> subfeatures;
    sensors_feature const *                 feature;
    int                                     nr = 0;
    while ((feature = sensors_get_features(cn_, &nr)) != 0) {
      const sensors_subfeature *const subfeature =
        sensors_get_subfeature(cn_, feature, SENSORS_SUBFEATURE_TEMP_INPUT);
      if (subfeature == NULL) {
        sensors_cleanup();
        throw std::runtime_error(
          "Failure on lmsensors::sensors_get_subfeature()");
      }
      subfeatures.push_back(subfeature);
    }
    return subfeatures;
  }

  const sensors_chip_name *const get_chip_name() const {
    if (sensors_init(NULL) != 0) {
      throw std::runtime_error("Failure on lmsensors::sensors_init(NULL)");
    }
    // TODO: This is wrong
    // TODO: Try to do automatic chip detection
    // Maybe look into subfeatures and see if there are any qualities that match
    // a chip
    sensors_chip_name const *cn;
    int                      c = 0;
    while ((cn = sensors_get_detected_chips(NULL, &c)) != 0) {
      if (strcmp(cn->prefix, "coretemp") == 0) {
        return cn;
      }
    }
    // Release resource before throw, destructor will not be called
    sensors_cleanup();
    // Haven't had the ability to test this on a non intel machine
    // The issue is detecting the chip_name that refers to the CPU and not
    // another component such as the chipset on the motherboard... maybe there
    // is another way...
    throw std::runtime_error("Supported chip_name prefix not detected");
  }

 private:
  // Order here matters, cn_ must be initialized before buildSubfeatures
  const sensors_chip_name *const                cn_;
  const std::vector<const sensors_subfeature *> subfeatures_;
};

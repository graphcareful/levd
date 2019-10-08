#ifndef LEVIATHAN_CONFIG_H
#define LEVIATHAN_CONFIG_H

#include "constants.h"
#include <functional>
#include <map>
#include <string>

#define DEFAULT_RED 0xFF0000

using LineFunction = std::function<int32_t(int32_t)>;

enum class TempSource { CPU, LIQUID };

inline TempSource stringToTempSource(const std::string &tss) {
  return tss == "liquid" ? TempSource::LIQUID : TempSource::CPU;
}

struct Point {
  int32_t x;
  int32_t y;
  Point(int32_t __x, int32_t __y) : x(__x), y(__y) {}
};

struct leviathan_config {
  // Fan/pump profile
  TempSource                      temp_source_{TempSource::CPU};
  std::map<int32_t, LineFunction> fan_profile_;
  std::map<int32_t, LineFunction> pump_profile_;

  // conky integration
  std::string conky_file_{kDefaultConkyFile};

  // Color settings
  uint32_t main_color_{DEFAULT_RED};

  // Interval settings
  uint32_t interval_{500};
};

leviathan_config parse_config_file(const char *const path);

#endif  // LEVIATHAN_CONFIG_H

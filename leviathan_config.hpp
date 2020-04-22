#pragma once
#include "constants.h"
#include <functional>
#include <map>
#include <string>

namespace levd {
using LineFunction = std::function<int32_t(int32_t)>;

enum class TempSource { Cpu, Liquid };

inline TempSource stringToTempSource(const std::string &tss) {
  return tss == "cpu" ? TempSource::Cpu : TempSource::Liquid;
}

struct leviathan_config {
  // Fan/pump profile
  TempSource                      temp_source_{TempSource::Liquid};
  std::map<int32_t, LineFunction> fan_profile_;
  std::map<int32_t, LineFunction> pump_profile_;

  // conky integration
  std::string conky_file_{kDefaultConkyFile};

  // Color settings
  uint32_t main_color_{DEFAULT_RED};

  // Interval settings
  uint32_t program_loop_interval_ms_{5000};
};

leviathan_config parse_config_file(const char *const path);
}  // namespace levd

#include "leviathan_config.h"
#include <exception>
#include <algorithm>
#include <glog/logging.h>
#include <yaml-cpp/yaml.h>

// TODO: Cleaner normalization
LineFunction slope_function(const Point &a, const Point &b) {
  const auto slope = (b.y - a.y) / (b.x - a.x);
  const auto bFac  = b.y - (slope * b.x);
  return [slope, bFac](int32_t x) {
    const auto newY = (slope * x) + bFac;
    // Normalize to a multiple of 5
    const auto evenDiff = newY % 5;
    return newY - evenDiff;  // hit it on the nose == 0
  };
}

std::map<int32_t, LineFunction> configure_profile(
  const YAML::Node &fan_profile) {
  CHECK(fan_profile.IsSequence()) << "Expecting a sequence of pairs";
  const auto point_compare = [](const Point &p, const Point &u) {
    return p.x > u.x;
  };
  std::vector<Point> dataPoints { Point(0, 30) };
  for (const auto &i : fan_profile.as<std::vector<std::vector<uint32_t>>>()) {
    CHECK(i.size() == 2) << "Expecting array of pairs for fan/pump profile";
    CHECK(i.back() % 5 == 0) << "Fan/pump profile values must be divisible by 5";
    dataPoints.emplace_back(i.front(), i.back());
  }
  dataPoints.emplace_back(100, 100);
  std::sort(dataPoints.begin(), dataPoints.end(), point_compare);
  std::map<int32_t, LineFunction> temp_to_slope;
  for (auto i = 0; i < dataPoints.size() - 1; ++i) {
    const Point &cur_pt = dataPoints[i];
    const Point &next_pt = dataPoints[i + 1];
    temp_to_slope[cur_pt.x] = slope_function(cur_pt, next_pt);
  }
  return temp_to_slope;
}

leviathan_config parse_config_file(const char *const path) {
  leviathan_config options;
  try {
    YAML::Node config     = YAML::LoadFile(path);
    options.temp_source_  = stringToTempSource(config["temperature_source"].as<std::string>());
    options.fan_profile_  = configure_profile(config["fan_profile"]);
    options.pump_profile_ = config["pump_profile"] ? configure_profile(config["pump_profile"]) : options.fan_profile_;
    options.main_color_   = config["main_color"].as<uint32_t>();
    options.interval_     = config["interval"].as<uint32_t>();
  } catch (std::exception &e) {
    LOG(FATAL) << "Yaml parsing error: " << e.what();
  }
  return options;
}

#ifndef LEVIATHAN_CONFIG_H
#define LEVIATHAN_CONFIG_H

#include <functional>
#include <map>

#define DEFAULT_RED 0xFF0000

using LineFunction = std::function<int32_t(int32_t)>;

struct Point {
  int32_t x;
  int32_t y;
  Point(int32_t __x, int32_t __y) : x(__x), y(__y) {}
};

struct leviathan_config {
  // Fan profile
  std::string temp_source_{"cpu"};
  std::map<int32_t, LineFunction> ftp_;

  // Color settings
  uint32_t main_color_{DEFAULT_RED};
  uint32_t alt_color_{DEFAULT_RED};

  bool enable_color_{false};
  bool enable_blink_{false};

  uint8_t blink_interval_{0};
  uint8_t alter_interval_{0};
  // Interval settings
  uint32_t interval_{500};
};

leviathan_config parse_config_file(const char *const path);

#endif  // LEVIATHAN_CONFIG_H

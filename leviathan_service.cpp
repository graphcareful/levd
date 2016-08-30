#include "leviathan_service.h"
#include "constants.h"  // #defines
#include "kraken_driver.h"
#include "leviathan_config.h"

#include <chrono>
#include <fstream>
#include <glog/logging.h>
#include <limits>
#include <thread>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

using namespace std::chrono_literals;

// Global kill all armed when SIGTERM is detected
volatile sig_atomic_t done = 0;
void term(int signum) { done = 1; }

/** *********** Private Interface ************** */

bool detect_kraken(libusb_device *device) {
  struct libusb_device_descriptor desc = {0};
  int err = libusb_get_device_descriptor(device, &desc);
  if (err != 0) {
    LOG(ERROR) << "Failed to get descriptor for generic device: "
               << libusb_error_name(err);
    return false;
  }
  return desc.idVendor == KRAKEN_X61_VENDOR
         && desc.idProduct == KRAKEN_X61_PRODUCT;
}

uint32_t next_speed(const std::map<int32_t, LineFunction> &fan_profile,
                    const uint32_t current_temp) {
  const auto slopeFn = fan_profile.upper_bound(current_temp)->second;
  return slopeFn(current_temp);
}

int file_is_modified(const char *path, time_t oldMTime) {
  struct stat file_stat;
  int         err = stat(path, &file_stat);
  LOG_IF(FATAL, err != 0) << "Error when attempting to stat " << path;
  return file_stat.st_mtime > oldMTime;
}

/** *********** Public Interface ************** */

libusb_device *leviathan_init(libusb_device **devices, ssize_t num_devices) {
  libusb_device *kraken_device = NULL;
  for (auto i = 0u; i < num_devices; ++i) {
    if (detect_kraken(devices[i])) {
      kraken_device = devices[i];
      break;
    }
  }
  return kraken_device;
}

void leviathan_start(libusb_device *kraken_device) {
  // Init Kraken, display diagnostics
  auto kd = std::make_unique<KrakenDriver>(kraken_device);
  LOG(INFO) << "Kraken Driver Initialized";
  LOG(INFO) << "Kraken Serial No: " << kd->getSerialNumber();

  // Open config file and handle to thermal monitor
  auto          config_opts = parse_config_file(kDefaultConfigFile);
  std::ifstream istream(kDefaultCPUTempFile);
  LOG_IF(FATAL, !istream.good()) << "Could not open: " << kDefaultCPUTempFile;
  uint32_t cpu_temp           = 0;
  uint32_t old_fan_speed      = 0;
  time_t   last_time_modified = std::numeric_limits<time_t>::min();

  // Init signal handler
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = term;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGQUIT, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  // Main program loop
  // 1. Read CPU thermal temperature & reseek to beginning
  // 2. Update config_options if config file was edited
  // 3. Set fan/pump speed according to CPU temp and given parameters
  // 4. Set color according to given setting
  // 5. Send update to Kraken, sleep for 0.5s and repeat
  while (!done) {
    // Grab latest cpu temperature
    istream >> cpu_temp;
    istream.seekg(0);

    // Grab latest parameters, if they've been changed
    if (file_is_modified(kDefaultConfigFile, last_time_modified)) {
      LOG(INFO)
        << "Detected modifications to config file, updating preferences...";
      config_opts        = parse_config_file(kDefaultConfigFile);
      last_time_modified = time(0);
    }

    // Based on parameters and current CPU temp, set desired kraken settings
    auto next = next_speed(config_opts.ftp_, (cpu_temp / 1000));
    if (abs(next - old_fan_speed) <= old_fan_speed * 0.1) {
      next = old_fan_speed;
    }
    VLOG(2) << "Current CPU Temperature: " << (cpu_temp / 1000) << "C";
    VLOG(2) << "Setting fan speed: " << next;
    kd->setFanSpeed(next);
    kd->setPumpSpeed(next);
    kd->setColor(config_opts.main_color_);
    const auto update = kd->sendKrakenUpdate();
    if (update.empty() == true) {
      LOG(WARNING)
        << "Bad update detected, attempting reconnection to kraken...";
      kd.reset(new KrakenDriver(kraken_device));
    }
    if (next != old_fan_speed) {
      LOG(INFO) << "Changed fan speed to " << update.find("fan_speed")->second
                << "rpm, with fan percentage at " << next
                << "% and current CPU temperature " << (cpu_temp / 1000) << "C";
      old_fan_speed = next;
    }
    std::this_thread::sleep_for(500ms);
  }
  istream.close();
}

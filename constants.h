#ifndef CONSTANTS_H
#define CONSTANTS_H

#define KRAKEN_X61_VENDOR 0x2433
#define KRAKEN_X61_PRODUCT 0xb200

#define KRAKEN_FAN_CODE 0x12
#define KRAKEN_PUMP_CODE 0x13
#define KRAKEN_COLOR_CODE 0x10

#define KRAKEN_INIT 0x0002
#define KRAKEN_BEGIN 0x0001

const char *const kDefaultCPUTempFile = "/sys/class/hwmon/hwmon0/temp1_input";
const char *const kDefaultConfigFile  = "/etc/leviathan/levd.cfg";

const unsigned char kDefaultColor[19] = {
  KRAKEN_COLOR_CODE, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x3c, 0x01, 0x01, 0x01, 0x00, 0x00,
  0x01, 0x00, 0x01
};

// 11 - interval
// 12 - interval
// 13 - enabled
// 14 - altBit
// 15 - blinking

#endif  // CONSTANTS_H

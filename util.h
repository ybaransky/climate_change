#ifndef UTIL_H
#define UTIL_H
#include <ESP8266WiFi.h>

extern  void          i2c_scan(void);
extern  uint32_t      seconds(void);
extern  char*         secondsToStr(uint32_t);
extern  char*         ipToStr(const IPAddress&);
extern  char*         networkAddressStr();
extern  char*         networkNameStr();

#endif

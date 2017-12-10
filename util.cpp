#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include "util.h"
#include "config.h"

char* networkAddressStr()
{
  static  char buffer[64];
  if (WiFi.status() != WL_CONNECTED)
    return ipToStr(WiFi.softAPIP());
    
  strcpy(buffer, WiFi.localIP().toString().c_str());
  return buffer;
}

char*  networkNameStr()
{
  static char buffer[64];
  if (WiFi.status() == WL_CONNECTED)
    strcpy(buffer, WiFi.SSID().c_str());
  else
    strcpy(buffer, gConfig.ap_ssid.c_str());
  return buffer;
}

void i2c_scan(void)
{
  byte error, address;

  Serial.println("Scanning i2c bus...");
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    switch (error)
    {
      case 0 : Serial.printf("I2C device found at address 0x%2X(hex) %d(dec)\n", address, address);  break;
      case 4 : Serial.printf("Unknown I2C error at address 0x%2X(hex) %d(dec)\n", address, address); break;
      default : break;
    }
  }
}

uint32_t  seconds(void) 
{
  static uint32_t low32=0, high32=0;
  uint32_t low = millis();
  if (low < low32) 
  { 
    high32++;
    low32 = low;
    return int32_t((uint64_t(high32) << 32 | low32) / 1000 );
  } 
  else 
  {
    low32 = low;
    return int32_t(low32 / 1000);
  }
}

char* secondsToStr(uint32_t s) 
{
  static uint32_t days = 86400; 
  static uint32_t hours = 3600; 
  static uint32_t minutes = 60;
  static char buffer[64];
  uint32_t sec = s;
  int dd = int( (sec                     ) / days );
  int hh = int( (sec - dd*days           ) / hours );
  int mm = int( (sec - dd*days - hh*hours) / minutes );
  int ss = sec - dd*days - hh*hours - mm*minutes;
  if (dd > 0) 
  {    
    sprintf(buffer,"%dd%02dh%0dm%02ds",dd,hh,mm,ss);
  } 
  else 
  {
    if (hh > 0) 
    {
      sprintf(buffer,"%dh%0dm%02ds",hh,mm,ss);
    } 
    else
    {
      if (mm > 0)
      {
        sprintf(buffer,"%dm%02ds",mm,ss);
      }
      else
      {
        sprintf(buffer,"%ds",ss);
      }
    }
  }
  String str = buffer;
  str.trim();
  strcpy(buffer,str.c_str());
  return buffer;
}

char* ipToStr(const IPAddress &ip)
{
  static char buffer[64];
  sprintf(buffer,"%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]); 

  /*
  buffer[0] = ipAddress[0];
  buffer[1] = '.';
  buffer[1] = ipAddress[1];
  String addr = String(ipAddress[0]) + String(".") +\
    String(ipAddress[1]) + String(".") +\
    String(ipAddress[2]) + String(".") +\
    String(ipAddress[3])  ;
  strcpy(buffer,addr.c_str());
*/
  return buffer; 
}


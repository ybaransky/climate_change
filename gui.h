#ifndef GUI_H
#define GUI_H

#include <arduino.h>
//#define USING_SSD1306
#ifdef USING_SSD1306 
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#else
#include "SH1106.h"  // alias for `#include "SH1106Wire.h"`
#endif
#include <ESP8266WiFi.h>
#include "sensor.h"

// 128 x 64
class Gui 
{
  private : 
#ifdef USING_SSD1306
    SSD1306   _display;
#else
    SH1106    _display;
#endif

  public :
    Gui(void) : _display(0x3C, D3, D4) {}
    void  init();
    void  drawSSIDPage(const char*, 
              const char* str1 = NULL,const char* str2 = NULL);
    void  drawSensorPage(const char lines[3][64]);
    void  drawTitle(const char* line, bool underline=true);
    void  drawGraphPage(const String&, const FloatReadings&, const FloatMinMax&);
    void  drawGraphTitle(const String lines[]);
    void  drawStatisticsPage();
    void  drawNewClientPage(const IPAddress&);
};
extern Gui  gGui;
#endif

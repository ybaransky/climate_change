#ifndef HTML_PAGES_H
#define HTML_PAGES_H
//#include "TimeClient.h"
//#include "thingspeak.h"
#include "sensor.h"

class HTMLPages {
  public :
    void sensorPage(void);
    void aboutPage(void);
    void configPage(void);
    void configSavePage(void);
    void configResetPage(void);
    
  private :
/*    Thingspeak&   _thingspeak;
    TimeClient&   _timeClient;
*/
};

extern  HTMLPages gHTMLPages;

#endif

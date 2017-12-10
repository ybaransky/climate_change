#ifndef CONFIG_H
#define CONFIG_H
#include "ArduinoJson.h"
#include "FS.h"

class Config
{

  public:
    int  display_interval;
    int  sensor_interval;
    int  thingspeak_interval;

    String  ap_ssid;
    String  ap_password;
    String  thingspeak_key;

  public:
    Config();
    
    bool  load();
    bool  save();
    void  reset();
    void  print();
    void  dir();
    bool  deleteFile();
    void  writeValue(const String&, const String&);

  private:
    void  writeJson( JsonObject& json, const char* name, String& value);
    void  readJson ( JsonObject& json, const char* name, String& value);
    void  writeJson( JsonObject& json, const char* name, int& value);
    void  readJson ( JsonObject& json, const char* name, int& value);

    bool    safeOpen(File&,const char*);
    String  _filename = "/config.json";
};

extern  Config  gConfig;

#endif

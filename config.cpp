#include <limits.h>

#include "config.h"

Config::Config()
{
  reset();
}

void Config::reset()
{
  display_interval    = 5;
  sensor_interval     = 15;
  thingspeak_interval = 600;

  ap_ssid        = "MeatGoneBad";
  ap_password    = "leslie1a";
  thingspeak_key = "UQ26QBJUD34PUVRV";     // www.thingspeak.com 
}

void  Config::writeValue( const String& name, const String& value0)
{
  String value(value0);
  value.trim();
  Serial.printf("writing %s with %s\n",name.c_str(), value.c_str());

  if (name.equals("display_interval"))
    display_interval = value.toInt();
  else if (name.equals("sensor_interval"))
    sensor_interval = value.toInt();
  else if (name.equals("thingspeak_interval"))
    thingspeak_interval = value.toInt();
  else if (name.equals("ap_ssid"))
    ap_ssid = value;
  else if (name.equals("ap_password"))
    ap_password = value;
  else if (name.equals("thingspeak_key"))
    thingspeak_key = value;
}

bool Config::deleteFile()
{
  SPIFFS.begin();
  if (!SPIFFS.exists(_filename))
  {
    Serial.printf("could not find %s to delete\n",_filename.c_str());
    return false;
  }
  if (!SPIFFS.remove(_filename))
  {
    Serial.printf("failed to remove file %s\n",_filename.c_str());
    return false;
  }
  Serial.printf("sucessfully removed %s\n",_filename.c_str());
  return true;
}

void Config::print()
{
  Serial.println("config parameters...");
  Serial.printf("['%s'] = %d\n"  , "display_interval",    display_interval);
  Serial.printf("['%s'] = %d\n"  , "sensor_interval",     sensor_interval);
  Serial.printf("['%s'] = %d\n"  , "thingspeak_interval", thingspeak_interval);

  Serial.printf("['%s'] = '%s'\n", "ap_ssid",        ap_ssid.c_str());
  Serial.printf("['%s'] = '%s'\n", "ap_password",    ap_password.c_str());
  Serial.printf("['%s'] = '%s'\n", "thingspeak_key", thingspeak_key.c_str());
}

bool Config::safeOpen(File& file, const char* mode)
{ 
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (!SPIFFS.begin()) 
  {
    Serial.println("failed to start FileSystem");
    return false;
  }
  Serial.println("mounted file system");
  
  if (!SPIFFS.exists(_filename)) 
  {
    if (mode[0] == 'r') // only for read only
    {
        Serial.printf("can't find file '%s'\n",_filename.c_str());
        return false;
    }
  }
  
  file = SPIFFS.open(_filename, mode);
  if (!file) 
  {
    Serial.printf("failed to open '%s' for more '%s'\n",_filename.c_str(),mode);
    return false;
  }
  Serial.printf("openned config file '%s' for mode '%s'\n",_filename.c_str(),mode);
  return true;
}
void  Config::writeJson( JsonObject& json, const char* name, int& value)    { json[name] = value; }
void  Config::readJson( JsonObject& json, const char* name, int& value)
{
  if (json.containsKey(name))
    value = json[name];
  else
    Serial.printf("key='%s' not found\n", name);
}

void  Config::writeJson( JsonObject& json, const char* name, String& value) { json[name] = value; }
void  Config::readJson( JsonObject& json, const char* name, String& value)
{
  if (json.containsKey(name))
    value = json[name].as<char*>();
  else
    Serial.printf("key='%s' not found\n", name);
}



bool Config::load(void) 
{
  File configFile;
  if (!safeOpen(configFile,"r"))
    return false;
 
  size_t size = configFile.size();
  Serial.printf("filesize=%d\n",size);
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  configFile.close();

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  json.prettyPrintTo(Serial);
  if (!json.success())
  {
    Serial.println("failed to parse json file");
    return false;
  }
  
  readJson(json, "display_interval",    display_interval);
  readJson(json, "sensor_interval",     sensor_interval);
  readJson(json, "thingspeak_interval", thingspeak_interval);

  readJson(json, "ap_ssid",        ap_ssid);
  readJson(json, "ap_password",    ap_password);
  readJson(json, "thingspeak_key", thingspeak_key);

  Serial.println("\nfinished loading config parameters");
  return true;
}

bool Config::save() 
{
  //save the custom parameters to FS
  Serial.println("saving config");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  writeJson(json, "display_interval",    display_interval);
  writeJson(json, "sensor_interval",     sensor_interval);
  writeJson(json, "thingspeak_interval", thingspeak_interval);

  writeJson(json, "ap_ssid",        ap_ssid);
  writeJson(json, "ap_password",    ap_password);
  writeJson(json, "thingspeak_key", thingspeak_key);

  json.prettyPrintTo(Serial); Serial.println ("");

  File configFile;
  if (!safeOpen(configFile,"w"))
    return false;  
  json.printTo(configFile);
  configFile.close();
  
  Serial.println("\nfinished saving config file");
  return true;
}

void Config::dir()
{
  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");
  int n=0;
  while (dir.next()) {
      Serial.print(dir.fileName());
      File f = dir.openFile("r");
      Serial.print("  size=");Serial.println(f.size());
      n++;
  }
  Serial.printf("found %d files\n",n);
}

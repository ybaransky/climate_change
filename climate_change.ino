#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include "config.h"
#include "html_pages.h"
#include "sensor.h"
#include "thingspeak.h"
#include "gui.h"
#include "wifi.h"
#include "util.h"

Config        gConfig;
SensorGroup   gSensorGroup;
Gui           gGui;
Wifi          gWifi;
HTMLPages     gHTMLPages;
Thingspeak    gThingspeak;
uint32_t      gIinitialFreeHeap;

std::unique_ptr<ESP8266WebServer> server;

void setup()
{
  Wire.begin(D3, D4);
  Serial.begin(115200);
//  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("Sensor comparsion");
  Serial.printf("Running on a %dMHz clock\n",ESP.getCpuFreqMHz());
  uint32_t t0 = ESP.getCycleCount();
  delay(100);
  uint32_t delta = ESP.getCycleCount()-t0;
  Serial.printf("100ms is %u cycles\n",delta);

  pinMode (BUILTIN_LED, OUTPUT) ;    // define LED as output interface

  if( !gConfig.load() )
    gConfig.save();
    
  gGui.init();
  i2c_scan();
  gSensorGroup.init();
  delay(200);
  gGui.drawSSIDPage(WiFi.SSID().c_str());
  gWifi.start();
}

void loop() {
  static bool firstTime = true; 
  static int lastSensor     = -100000;
  static int lastDisplay    = -100000;
  static int lastThingspeak = -100000;
  int now = millis();
  long mem0,mem1;

  if (firstTime)
  {
    firstTime = false;
 //   gInitialFreeHeap = ESP.getFreeHeap();
  }

  if (gWifi.num_clients_changed())
  {
    Serial.printf("number of clients=%d\n",gWifi.num_clients());
    Serial.print("last client IPaddres ");
    Serial.println(gWifi.last_client_ipaddress());
  }

  if (now - lastSensor > 1000*gConfig.sensor_interval) 
  {
    lastSensor = now;
    mem0 = ESP.getFreeHeap();
    digitalWrite(LED_BUILTIN, LOW);
    gSensorGroup.read();
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.printf("reads took   =%d msec\n",millis()-now);
    Serial.printf("sensor leaked=%d bytes\n",mem0 - ESP.getFreeHeap());
    Serial.printf("esp8266 addr =%s\n",networkAddressStr());
    Serial.printf("max ping time=%d msec\n", gWifi.getMaxPingTime());
  }
  
  if (now - lastDisplay > 1000*gConfig.display_interval)
  {
    lastDisplay = now;

    static int page = 0;
    static int num_clients = 0;
    char lines[3][64];
    String  title;

    long mem0 = ESP.getFreeHeap();
    int clients = gWifi.num_clients();
    if (clients != num_clients) 
    {
        num_clients = clients;
        if (clients)
          gGui.drawNewClientPage(gWifi.last_client_ipaddress());
    }
    else
    {
      switch (page)
      {
        case 0:
          gSensorGroup.getDisplayLines(Sensor::Data::TEMPERATURE, lines);
          gGui.drawSensorPage(lines);
          break;
        case 1:
          title = "Temp";
          gGui.drawGraphPage(title, gSensorGroup.temperatures(), gSensorGroup.temperatureGlobalMinMax());
          break;
        case 2:
          title = "DeltaT";
          gGui.drawGraphPage(title, gSensorGroup.deltas(), gSensorGroup.deltaGlobalMinMax());
          break;
        case 3:
          gSensorGroup.getDisplayLines(Sensor::Data::HUMIDITY, lines);
          gGui.drawSensorPage(lines);
          break;
        case 4:
          gGui.drawStatisticsPage();
          break;        
      }
      page = (page+1) % 5;
    }
    mem1 = ESP.getFreeHeap();
    Serial.printf("graph %d  leaked=%d\n",page,mem0-mem1);
  }

  if (now - lastThingspeak > 1000*gConfig.thingspeak_interval)
  {
    lastThingspeak = now;
    gThingspeak.update();
    gWifi.resetMaxPingTime();
  }

  server->handleClient();
}

#include <algorithm>
#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266Ping.h>
#include <ESP8266mDNS.h>
#include "config.h"
#include "html_pages.h"
#include "wifi.h"
#include "gui.h"

extern "C" {
#include<user_interface.h>
}

void handleNotFound(){
//  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server->uri();
  message += "\nMethod: ";
  message += (server->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server->args();
  message += "\n";
  for (uint8_t i=0; i<server->args(); i++){
    message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
  }
  server->send(404, "text/plain", message);
//  digitalWrite(led, 0);
}

void Wifi::apConfigCallback(WiFiManager* mgr)
{
  //  gui.drawConfigPage(myWiFiManager);
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  gGui.drawSSIDPage(NULL, "Failure... ","Starting access poiunt");
  delay(1000);
  gGui.drawSSIDPage(mgr->getConfigPortalSSID());

  //  Serial.println(myWiFiManager->getConfigPortalSSID());
  //  installHandlers();
}

int Wifi::getNetworkStrength(void)
{
  if (WiFi.status() == WL_CONNECTED)
    return -WiFi.RSSI();
  return 0;
}

void Wifi::resetMaxPingTime(void) { _max_ping_time = 1;}
int  Wifi::getMaxPingTime(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    IPAddress ip(8,8,8,8);
    if (Ping.ping(ip))
    {
      int ms = Ping.averageTime();
      if (_max_ping_time) // 0 means sticky failure
        _max_ping_time = std::max(_max_ping_time, ms);
    }
    else
    {
      _max_ping_time = 0;
    }
  }
  return _max_ping_time;
}

void  Wifi::apHandlersCallback(WiFiManager* mgr)
{
  Serial.printf("installing handlers with %x\n",server.get());
  if (server.get()) 
  {
    server->on("/",           std::bind(&WiFiManager::handleRoot,     mgr));
    server->on("/home",       std::bind(&WiFiManager::handleRoot,     mgr));
    server->on("/wifi",       std::bind(&WiFiManager::handleWifi,     mgr, true));
    server->on("/0wifi",      std::bind(&WiFiManager::handleWifi,     mgr, false));
    server->on("/wifisave",   std::bind(&WiFiManager::handleWifiSave, mgr));
    server->on("/info",       std::bind(&WiFiManager::handleInfo,     mgr));
    server->on("/reset",      std::bind(&WiFiManager::handleReset,    mgr));
    server->on("/fwlink",     std::bind(&WiFiManager::handleRoot,     mgr));

    server->on("/configsave",  std::bind(&HTMLPages::configSavePage , &gHTMLPages));
    server->on("/configreset", std::bind(&HTMLPages::configResetPage ,&gHTMLPages));
    server->on("/config",      std::bind(&HTMLPages::configPage,      &gHTMLPages));
    server->on("/sensor",      std::bind(&HTMLPages::sensorPage,      &gHTMLPages));
    server->on("/about",       std::bind(&HTMLPages::aboutPage,       &gHTMLPages));
    server->on("/",            std::bind(&HTMLPages::sensorPage,      &gHTMLPages));

    server->onNotFound(handleNotFound);
 
  }
  else
  {
    Serial.println("yikes,,, server.get() is NULL");
  }
}


void  Wifi::start(void)
{
  resetMaxPingTime();
  // start the Wifi stuff
  Serial.print("starting WiFi... ");
  Serial.print("previous ssid: ");
  Serial.println(WiFi.SSID());
  WiFi.printDiag(Serial);

  _wifiManager = new WiFiManager();
  _wifiManager->setAPCallback(         Wifi::apConfigCallback );
  _wifiManager->setAPHandlersCallback( Wifi::apHandlersCallback );
  _wifiManager->addButton("/sensor", "Sensor Data");
  _wifiManager->addButton("/config", "Configure Device");
  _wifiManager->addButton("/wifi",   "Configure Wifi");
  _wifiManager->addButton("/info",   "Device Info");
  _wifiManager->addButton("/about",  "About");

  if (!_wifiManager->autoConnect( gConfig.ap_ssid.c_str(), gConfig.ap_password.c_str()))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  } 
  else 
  {
    // if we are here, then we connected to an AP
    Serial.printf("resetting the server using ip:%s\n",WiFi.localIP().toString().c_str());
    server.reset(new ESP8266WebServer(WiFi.localIP(), 80));
    Serial.printf("new server address %x\n", server.get());
    
    if (MDNS.begin(gConfig.ap_ssid.c_str()) )
    {
      Serial.println("MDNS responder started");
    }
    
    // setup the pages to display
    Wifi::apHandlersCallback(_wifiManager);
    server->begin();
    Serial.println("HTTP server started");
    Serial.println(WiFi.localIP());
  }
}

uint8_t Wifi::num_clients()
{
  return uint8_t(wifi_softap_get_station_num()); // Count of stations which are connected to ESP8266 soft-AP
}

bool Wifi::num_clients_changed()
{
  // we do not return ture until we actually change the number of clients
  // and we have a station_info structure allocated.
  // there is some serious timing issue, so we chose this
  static uint8_t N_CLIENTS = 0;
  uint8_t nclients;

  nclients = num_clients();
  if (N_CLIENTS != nclients)
  { 
    int n=0;
    struct station_info* stat_info = wifi_softap_get_station_info();
    while (stat_info != NULL) 
    {
      n++;  
      stat_info = STAILQ_NEXT(stat_info, next);
    }
    
    if (n == nclients) 
    {
      N_CLIENTS = nclients;
      return true;
    }
  }
  return false;
}

IPAddress Wifi::last_client_ipaddress()
{ 
  struct station_info* stat_info = wifi_softap_get_station_info();
  IPAddress ipaddr = {0,0,0,0};
  while (stat_info != NULL) 
  {
    ipaddr = (&stat_info->ip)->addr;;
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  return ipaddr;
}

#ifndef WIFI_H
#define WIFI_H
#include "wifi_manager.h"

class Wifi 
{
private :
  const byte  DNS_PORT = 53;

  WiFiManager*      _wifiManager;

  void    mdnsStart();
  bool    captivePortal();
  void    installHandlers();    

  int     _max_ping_time;

public:
  static void    apConfigCallback(WiFiManager*);
  static void    apHandlersCallback(WiFiManager*);

  void      start(void);
  uint8_t   num_clients();
  bool      num_clients_changed();
  IPAddress last_client_ipaddress();
  int       getNetworkStrength(void);
  void      resetMaxPingTime(void);
  int       getMaxPingTime(void);
};

extern  Wifi  gWifi;

#endif

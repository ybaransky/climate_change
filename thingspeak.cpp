//#include <Arduino.h>
#include <algorithm>
#include <ESP8266WiFi.h>
#include "thingspeak.h"
#include "sensor.h"
#include "config.h"
#include "wifi.h"

Thingspeak::Thingspeak() : _updates(0) {}

bool Thingspeak::update(void)
{
    static const String host("api.thingspeak.com");
    Serial.printf("--> Writing to thingspeak  time %dsecs<--\n",millis()/1000);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(host.c_str(), Thingspeak::HTTP_PORT)) {
      Serial.println("connection failed");
      return false;
    }
    if (gConfig.thingspeak_key.length()==0)
      return false;

    // int rssi = gWifi.getNetworkStrength();
    int ping_time = gWifi.getMaxPingTime();
    int measurements = gSensorGroup.measurements();

    String  strs[10];
    int n=0;
    strs[n++] = String(gSensorGroup.bme280().temperature().str2());
    strs[n++] = String(gSensorGroup.delta().temperature().str2());
    strs[n++] = String(gSensorGroup.bme280().humidity().str2());
    strs[n++] = String(gSensorGroup.delta().humidity().str2());
    strs[n++] = String(gSensorGroup.bme280().pressure().str2());
    strs[n++] = String(ESP.getFreeHeap());
    strs[n++] = String(measurements);
    strs[n++] = String(ping_time);
    Serial.printf("Temperature: BME280=%s  Delta=%s\n",strs[0].c_str(), strs[1].c_str());
    Serial.printf("Humidity:    BME280=%s  Delta=%s\n",strs[2].c_str(), strs[3].c_str());
    Serial.printf("Pressure:    BME280=%s\n",strs[4].c_str());
    Serial.printf("FreeHeap:    memory=%s\n",strs[5].c_str());
    Serial.printf("Measureents:       =%s\n",strs[6].c_str());
    Serial.printf("Ping Time        ma=%s\n",strs[7].c_str());
    //
    // We now create a URI for the request
    String url = "/update?api_key=" + gConfig.thingspeak_key;
    url += "&field1=" + strs[0];
    url += "&field2=" + strs[1];
    url += "&field3=" + strs[2];
    url += "&field4=" + strs[3];
    url += "&field5=" + strs[4];
    url += "&field6=" + strs[5];
    url += "&field7=" + strs[6];
    url += "&field8=" + strs[7];
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(10);
    while(!client.available()){
      delay(100);
      Serial.print(".");
    }
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    _updates++;
    Serial.println();
    Serial.println("sucess... closing connection");
}

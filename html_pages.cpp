#include <algorithm>
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "html_pages.h"
#include "config.h"
#include "util.h"

extern std::unique_ptr<ESP8266WebServer> server; //yurij

static int requests = 0;
static const String NL("\r\n");
static const String EMPTY("");
static const char STYLE_BUTTON[] PROGMEM = "button {border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:40%;}" ;

static String header(const char* header) {
  String html("");
  html = "<h3 style='text-align:center; font-weight:bold'>" + String(header) + "<br></h3>" + NL;
  return html;
}

static void send0(const String& page) {
  requests++;
  Serial.printf("%d )\n",millis());
  Serial.println(page);
  if (server.get())
    server->send(200, "text/html", page.c_str());
  else
    Serial.println("send0: no webserver found");
}

static String wrapper(const char* title, const String& body, const String& style) {
  String page;
  page = "";
  page += "<!DOCTYPE html>" + NL;
  page += "<html lang='en'>" + NL;
  page +=   "<head>" + NL;
  page +=     "<meta charset='UTF-8'>" + NL;
  page +=     "<meta name='author' content='Yurij Baransky'>" + NL;
  page +=     "<meta name='viewport' content='width=device-width, initial-scale=1.0'>" + NL;
  page +=     "<title>" + String(title) + "</title>" + NL;
  if (style.length())
    page += style;
  page +=   "</head>" + NL;
  page +=   "<body><div style='text-align:center; display=inline-block; min-width=260px;'>" + NL;
  page +=      body + NL;
  page +=   "</div></body>"+ NL;
  page += "</html>" + NL;
  return page;
}

static void send(const char* title, const String& body, const String& style=EMPTY) {
  requests++;
  String page = wrapper(title, body, style);
  Serial.printf("%d )\n",millis());
  Serial.println(page);
  if (server.get())
    server->send(200, "text/html", page.c_str());
  else
    Serial.println("send: no webserver found");
}

void HTMLPages::sensorPage(void) {
  int t0 = millis();
  int nowSeconds = millis()/1000;
  char  buffer[128];
  String style ="";
  style += "<meta http-equiv='refresh' content='10'>" + NL;  // add this only here
  style += "<style>" + NL;
  style +=   "table {border-collapse: collapse;}" + NL;
  style +=   "table, th, td {border: 2px solid blue;}" + NL;
  style +=   "th, td {padding: 1px; text-align: center;}" + NL;
 // style += "th {font-weight: bold; font-size: larger;}" + NL;
  style +=   STYLE_BUTTON + NL;
  style += "</style>" + NL;
  
  static String body;
  body = header("Yurij's Sensor Testing Page");
  body += "<table width='100%'>" + NL;
  body +=   "<tr><th></th>";

  Sensors& sensors = gSensorGroup.sensors();
  for( Sensor* sensor : sensors) {
    sprintf(buffer,"<th>%s</th>", sensor->htmlname());
    body += buffer;
  }
  body +=   "</tr>" + NL;

  body += "<tr><td>Temperature (&deg;F) </td>";
  for( Sensor* sensor : sensors) {
    sprintf(buffer,"<td>%s</td>", sensor->temperature().str());
    body += buffer;
  }
  body += "</tr>" + NL;

  body += "<tr><td>Humidity (%)</td>";
  for( Sensor* sensor : sensors) {
    sprintf(buffer,"<td>%s</td>", sensor->humidity().str());
    body += buffer;
  }
  body += "</tr>" + NL;

  sprintf(buffer,"<tr><td>Pressure (in-Hg)</td><td>%s</td><td></td><td></td></tr>", sensors[0]->pressure().str());
  body += buffer;
  body += NL;

  body += "</table>" + NL;
  body += "<p style='text-align:right; font-size:smallest'>" + NL;
//  body += "<br>" + NL;
  sprintf(buffer, "Samples: %d<br>", gSensorGroup.measurements());
  body += buffer + NL;
  
  sprintf(buffer, "Uptime: %s<br>", secondsToStr( nowSeconds ));
  body += buffer + NL;

  /*
  body += "Uptime: "       + globals.getFormattedBootTime() + "<br>" + NL;
  body += "Current time: " + _timeClient.getFormattedTime() + "<br>" + NL;
  */
//  body += "</p>" + NL;  
//  body += "<br>" + NL;
  body += "<form action='/home' method='get'><button>Home</button></form>";
//<FORM> <INPUT TYPE="button" onClick="history.go(0)" VALUE="Refresh"> </FORM>
   body += "<br>" + NL;

  // the large table of values

  body += "<table width='100%'>" + NL;
  body += "<tr>";
  sprintf(buffer,"<th></th><th>Time</th><th>%s</th><th>%s</th>", sensors[0]->htmlname(),sensors[2]->htmlname());
  body += buffer;
  body += "</tr>" + NL;

  // need to go in reverse
  const Floats& temperatures = gSensorGroup.temperatures().values();
  const Floats& deltas = gSensorGroup.deltas().values();
  const Times& times = gSensorGroup.times().values();
  int lastSeconds = times.back();
  int n= temperatures.size();
  for(int j,i=0; i < std::min(n,20); i++)
  {
    j = n-1-i;
    const char* temp  = temperatures[j].str2();
    const char* delta = deltas[j].str2();
    const char* secs  = secondsToStr(lastSeconds - times[j]);
    body += "<tr>";
    sprintf(buffer,"<td>%d</td><td>%s</td><td>%s</td><td>%s</td>", i+1, secs, temp, delta);
    body += buffer;
    body += "</tr>" + NL;
  }
  body += "</table>" + NL;
  int t1 = millis() - t0;
  send("Yurij's Sensor Page", body, style);

  Serial.printf("page size... %d   msec=%d\n",body.length(),t1);
}

void HTMLPages::aboutPage(void) {
  String style = "";
  style += "<style>" + NL;
  style +=   "table {border-collapse: collapse;}" + NL;
  style +=   "table, th, td {border: 2px solid blue;}" + NL;
  style +=   STYLE_BUTTON + NL;
  style += "</style>";

  String body = header("About");  
  body += "<p style='text-align:center'>"\
"If sensors from reputable manufacturor's can't agree on the temperature to within a degree or two, "\
"or be calibrated so they agree over a range of temperatures, then how can anyone talk "\
"about the 'temperature', around the globe, now or in 10 years? I can't even measure it right "\
"in front of me, right now. The point is that ALL of Science has error bars, sometimes large ones. "\
"They need to be included. "\
"<a href='http://calteches.library.caltech.edu/51/2/CargoCult.htm'>Omitting data</a> that weakens your "\
"arguement is not Science. It is lobbying.<br>No more frauds.";

  body += "<p style='text-align:right'><a href='http://www.askyurij.com'>www.askyurij.com</a></p>" + NL;
  body += "<br><br>" + NL;
  body += "<form action='/home' method='get'><button>Home</button></form>";
  send("About", body, style);
}

String configPageTitle(const char* title) { return String("<title>") + title + String("</title>") + NL; }

String configPageRow(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  //row += "<td style='text-align:right; font-weight:bold'>" + String(name) + "</td><td style='text-align:left'>" + value + "</td>";
  row += "<td class='right bold'>" + String(desc) + "</td><td class='left grey'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}

String configPageInputRow(const char* desc, const String& value) {
  String row("");
  row += "<tr>";
  //row += "<td style='text-align:right; font-weight:bold'>" + String(name) + "</td><td style='text-align:left'>" + value + "</td>";
  row += "<td class='right border bold grey'>" + String(desc) + "</td><td class='left border'>" + value + "</td>";
  row += "</tr>" + NL;
  return row;
}
String configPageInputText(const char* id, String& value, int size) 
{
  String input = NL;
  input += "<input ";
  input += "type='text' ";
  input += "id='"          + String(id)    + "' ";
  input += "name='"        + String(id)    + "' ";
  input += "placeholder='" + String(value) + "' ";
  input += "maxlength="    + String(size) + " ";
  input += "size="         + String(size) + " ";
  input += ">";
  return input;
}
String configPageInputRate(const char* id, int& value, int size) 
{
  String str(value);
  String input = "every &nbsp";
  input += "<input ";
  input += "type='text' ";
  input += "id='"          + String(id)   + "' ";
  input += "name='"        + String(id)   + "' ";
  input += "placeholder='" + String(str)  + "' ";
  input += "maxlength="    + String(size) + " ";
  input += "size="         + String(size) + " ";
  input += "min='10' max='3600'";
  input += "> sec";
  return input;
}

void HTMLPages::configPage(void) {
  String page = "";
  page += "<!doctype html>"    + NL;;
  page +=   "<html lang='en'>" + NL;;
  page +=     "<head>" + NL;
  page +=       "<meta name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>" +configPageTitle("configuration");
  page +=       "<style type='text/css'>" + NL;
//  page += "input {padding:1px;font-size:1em; background-color=lightblue}"  + NL; 
//  page += ".c{text-align: center;}" + NL;
//  page += "input{width:95%; margin-bottom: 10px}" + NL;
//  page += "div{padding:5px;font-size:1em;}"  + NL; 
//  page += "body{text-align: center;font-family:verdana;}" + NL;
//  page += ".q{float: right;width: 64px;text-align: right;}" + NL;
// page +=         "table    {margin-left:auto; margin-right:auto; border-collapse:collapse;}" + NL;
//  page +=         "html, body  {width: 100%}" + NL;
  page +=         "table       {text-align:center; border-collapse:collapse; width=90%; margin=0px auto;}" + NL;
  page +=         "th,td       {width:50%; padding-left: 5px; padding-right: 5px; border:1px solid blue}" + NL;
  page +=         "td.noborder {border: 0px;           }" + NL;
  page +=         "td.bold     {font-weight:bold;}" + NL;
  page +=         "td.left     {text-align:left;}"  + NL;
  page +=         "td.right    {text-align:right;}" + NL;
  page +=         "td.grey     {background-color:#f2f2f2;}" + NL;
  page +=         STYLE_BUTTON + NL;
  page +=         "input[type='text'], input[type='number'] {font-size:100%; border:2px solid red}"  + NL; 
  page +=       "</style>" + NL;
  page +=     "</head>" + NL;
  page +=     "<body>"  + NL;  
  page +=     "<div style='text-align:center; min-width:260px;'>";
//  page +=     "<div style='text-align:center; display:inline-block; min-width:260px;'>";
//  page +=     "<div style='text-align:center; min-width:260px;'>" + NL;
//  page +=     "<div style='display:block: min-width:260px;'>" + NL;

  // 
  // title of the pag
  //
  page += header("Device Configuration");

  //
  // wrap this in a form
  //
  page += "<form method='get' action='configsave'>" + NL;

  //
  // the table
  //
  //page += "<table width='100%' style='margin: 0px'>" + NL;
  page += "<table>" + NL;
  String input_row;

  input_row = configPageInputText("ap_ssid", gConfig.ap_ssid, 16); 
  page += configPageInputRow("AP name", input_row);

  input_row = configPageInputText("ap_password", gConfig.ap_password, 16); 
  page += configPageInputRow("AP password", input_row);

  input_row = configPageInputText("thingspeak_key", gConfig.thingspeak_key, 16); 
  page += configPageInputRow("Thingspeak key", input_row);

  input_row = configPageInputRate("thingspeak_interval", gConfig.thingspeak_interval, 3);
  page += configPageInputRow("Publish", input_row);

  input_row = configPageInputRate("display_interval", gConfig.display_interval, 3);
  page += configPageInputRow("Display", input_row);

  input_row = configPageInputRate("sensor_interval", gConfig.sensor_interval, 3);
  page += configPageInputRow("Sample", input_row);

  page += "</table>" + NL;
  page += "<br><br>";
  page += "<button type='submit' value='Save' >Save</button>"  + NL;
  page += "</form>" + NL;
  page += "<br><form action='/configreset' method='get'><button>Defaults</button></form>";
  page += "<br>";
  page += "<br><form action='/home' method='get'>  <button>Home</button></form>";
    
  //
  // the end
  //
  page +=   "</div>";
  page += "</body></html>";
  Serial.printf("page length=%d\n",page.length());
  send0(page);  
}

void HTMLPages::configResetPage(void) 
{
  gConfig.reset();
  configPage();
}

void HTMLPages::configSavePage(void) 
{
  if (server.get()) 
  {
    String ap_ssid             = server->arg("ap_ssid").c_str();    
    String ap_password         = server->arg("ap_password").c_str();    
    String thingspeak_key      = server->arg("thingspeak_key").c_str();
    String display_interval    = server->arg("display_interval").c_str();
    String sensor_interval     = server->arg("sensor_interval").c_str();
    String thingspeak_interval = server->arg("thingspeak_interval").c_str();

    String page = "<style type='text/css'>" + NL;
    page +=         STYLE_BUTTON + NL;
    page +=       "</style>" + NL;
    
    page += String("<h1>") + 
      "AP ssid="        + ap_ssid + "<br>\n" +
      "AP password="    + ap_password + "<br>\n" +
      "Thingspeak key=" + thingspeak_key + "<br>\n" +
      "Display="        + display_interval + "<br>\n" +
      "Sensor="         + sensor_interval + "<br>\n" +
      "Thingspeak="     + thingspeak_interval +"<br\n" +
      "</h1>\n";
      
    String ipaddr = networkAddressStr();
    String script = "<script>";
    script += "var timer = setTimeout(function() { window.location=";
    script += "'http://" + ipaddr + "'}, 3000);";
    script += "</script>" + NL;

    page += script;
    
    Serial.println(page);
    for(int i=0; i<server->args();i++)
    {
      if (server->arg(i).length()) 
        gConfig.writeValue(server->argName(i), server->arg(i) );
    } 
    page += "<br><form action='/home' method='get'><button>Home</button></form>\n";
  
//    gConfig.print();
    gConfig.save();
    server->send(200, "text/html", page);
  } 
  else 
  {
    Serial.println("configSave: no webserver found...");
  }
}

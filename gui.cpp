#include "gui.h"
#include "sensor.h"
#include "util.h"
#include "config.h"

void  Gui::init(void)
{
  Serial.println("starting display...");
  Serial.printf("width=%d  height=%d\n",DISPLAY_WIDTH, DISPLAY_HEIGHT);
  delay(100);  // need for async junk on the display
  _display.init();
  _display.flipScreenVertically();
  _display.clear();
  _display.setFont(ArialMT_Plain_16);
  _display.setTextAlignment(TEXT_ALIGN_CENTER);
  _display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2-16, "Climate Change..."); 
  _display.display();
}

void  Gui::drawSSIDPage(const char* ssid, 
      const char* statusStr1, const char* statusStr2)
{
  if (!statusStr1)
  {
    _display.clear();
    _display.setFont(ArialMT_Plain_16);
    _display.setTextAlignment(TEXT_ALIGN_CENTER);
    drawTitle("Connecting to");
    _display.setFont(ArialMT_Plain_10);
    _display.setTextAlignment(TEXT_ALIGN_LEFT);
    _display.drawString(0, 23, ssid);
  }
  else
  {
    _display.setTextAlignment(TEXT_ALIGN_LEFT);
    _display.drawString(0, 35, statusStr1);
    _display.drawString(0, 45, statusStr2);
  }
  _display.display();
}

void  Gui::drawTitle(const char* title, bool underline)
{
  _display.setFont(ArialMT_Plain_16);
  _display.setTextAlignment(TEXT_ALIGN_CENTER);
  _display.drawString(DISPLAY_WIDTH/2, 0, title); 
  // underline it
  if (underline)
  {
    int16_t width = _display.getStringWidth(title, strlen(title));  
    int16_t offset = (DISPLAY_WIDTH - width)/2;
    int16_t y = 16;
    _display.drawLine(offset, y, offset + width, y);  
  }
}

void Gui::drawSensorPage(const char lines[3][64]) 
{ 
  _display.clear();
   drawTitle(lines[0]);  
  _display.setFont(ArialMT_Plain_16);
  _display.setTextAlignment(TEXT_ALIGN_LEFT);
  _display.drawString(0, 23, lines[1]);  
  _display.drawString(0, 40, lines[2]); 
  _display.display();
}


void  Gui::drawGraphTitle(const String lines[])
{
  String title = lines[0] + " " + lines[1] + "F";
  _display.setFont(ArialMT_Plain_10);
  _display.setTextAlignment(TEXT_ALIGN_LEFT);
  _display.drawString(0, 0, title);

  String range = "[ " + lines[2] + ", " + lines[3] + " ]";
  _display.setFont(ArialMT_Plain_10);
  _display.setTextAlignment(TEXT_ALIGN_RIGHT);
  _display.drawString(DISPLAY_WIDTH,0, range); 
}

void  Gui::drawGraphPage(const String& title, const FloatReadings& readings, const FloatMinMax& gMinMax)
{
  const Floats& values = readings.values();

  // graph bounds
  FloatMinMax vMinMax = readings.minmax();
  float vmin = 0.1* std::floor(10.0*vMinMax.min().value());
  float vmax = 0.1* std::ceil( 10.0*vMinMax.max().value());
  float scale  = 1.0 / (vmax - vmin);

  uint8_t LEFT_OFFSET = 25;
  uint8_t BOT_OFFSET = 1;
  uint8_t TOP_OFFSET = 17;
  uint8_t RIGHT_OFFSET = 0;
  uint8_t RECT_WIDTH = 1;  // base of each rectangle

  uint8_t GRAPH_BASE   = DISPLAY_HEIGHT - BOT_OFFSET;
  uint8_t GRAPH_WIDTH  = DISPLAY_WIDTH-LEFT_OFFSET - RIGHT_OFFSET;
  uint8_t GRAPH_HEIGHT = DISPLAY_HEIGHT - TOP_OFFSET - BOT_OFFSET;

  _display.clear();
  String lines[10];
  lines[0] = title;
  lines[2] = gMinMax.min().str();
  lines[1] = values.back().str();
  lines[3] = gMinMax.max().str();
  drawGraphTitle(lines);

  // x, y axis
  bool    hasZero = (vmin*vmax < 0.0);
  uint8_t zeroOffset = hasZero ? uint8_t(std::abs(vmin * scale * GRAPH_HEIGHT)) : 0;
  _display.drawLine(LEFT_OFFSET, GRAPH_BASE-zeroOffset, DISPLAY_WIDTH, GRAPH_BASE-zeroOffset); //x-axis
  _display.drawLine(LEFT_OFFSET, TOP_OFFSET,            LEFT_OFFSET,   GRAPH_BASE); //y-axis

  for(int i=0; i < values.size(); i++)
  {
    uint8_t h = uint8_t((values[i].value() - vmin) * scale * GRAPH_HEIGHT);
    int     x = LEFT_OFFSET + i*RECT_WIDTH;

    int bar =2;
    if (zeroOffset > h)
    {
      if(RECT_WIDTH==1)
//        _display.drawVerticalLine(1+x, GRAPH_BASE - zeroOffset, zeroOffset - h);
        _display.drawVerticalLine(1+x, GRAPH_BASE - h - bar, bar) ;//zeroOffset - h);

      else
        _display.fillRect(1+x, GRAPH_BASE - zeroOffset, RECT_WIDTH, zeroOffset - h);
    }
    else
    {
      if (RECT_WIDTH==1)
        _display.drawVerticalLine(1+x, GRAPH_BASE - h - bar, bar); // h - zeroOffset);        
      else
        _display.fillRect(1+x, GRAPH_BASE - h,     RECT_WIDTH, h - zeroOffset);
    }
      
//    Serial.printf("%d) x=%d  height=%d  v=%s  t=%d\n", i, x, h, values[i].str().c_str(), times[i]);
  }
  char buf1[32];
  char buf2[32];
  dtostrf(vmin,5,1,buf1);
  dtostrf(vmax,5,1,buf2);
  _display.setFont(ArialMT_Plain_10);
  _display.setTextAlignment(TEXT_ALIGN_LEFT);
  _display.drawString(0, TOP_OFFSET       , buf2);
  _display.drawString(0, DISPLAY_HEIGHT-10, buf1);  
  
  _display.display();
}

void  Gui::drawStatisticsPage(void)
{
  char line[10][64];
  int n=0;
  int secs = gConfig.sensor_interval;
  sprintf(line[n++], "  uptime: %s",  secondsToStr(seconds()));
  sprintf(line[n++], "samples: %d",   gSensorGroup.measurements());
  sprintf(line[n++], "    graph: %s", secondsToStr(100*secs));
  sprintf(line[n++], "   period: %s", secondsToStr(secs)); 
  sprintf(line[n++], "   ipaddr: %s", networkAddressStr());
  sprintf(line[n++], " network: %s",  networkNameStr());
  
  _display.clear();
  _display.setFont(ArialMT_Plain_10);
  _display.setTextAlignment(TEXT_ALIGN_LEFT);
  for(int i=0; i<n; i++)
    _display.drawString(1, 0 + i*10, line[i]);
  _display.display();
}

void  Gui::drawNewClientPage(const IPAddress& ipaddr)
{
    char  line[64];
    sprintf(line,"ipaddr: %s",ipToStr(ipaddr));
     _display.clear();    
    drawTitle("New Client", true);
    _display.setFont(ArialMT_Plain_10);
    _display.setTextAlignment(TEXT_ALIGN_LEFT);
    _display.drawString(0, 20, line);
    _display.display();
}


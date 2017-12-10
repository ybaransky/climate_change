#include <math.h>
#include <algorithm>
#include "sensor.h"

Float::Float(float f)
{
  set(f);
}

Float& Float::operator=(const Float& rhs) 
{
  if (this != &rhs)
  {
    _float = rhs._float;
    strncpy(_str, rhs._str, Float::STR_SIZE);
  }
  return *this;
}
bool  operator<(const Float& lhs, const Float& rhs)
{
  return lhs._float < rhs._float;
}

void  Float::set(float f, int decimals)
{ 
  String tmp;
  _float = f; 
  dtostrf(f, 6+decimals, decimals, _str);
  tmp = _str;
  tmp.trim();
  strcpy(_str,tmp.c_str());
  
  decimals = 2;
  dtostrf(f, 6+decimals, decimals, _str2);
  tmp = _str2;
  tmp.trim();
  strcpy(_str2, tmp.c_str());
}

//////////////////////////////////////////////////////////////

Sensor::Data::Data(Data::Type t) : _type(t), _blank(false)
{
  switch(t)
  {
    case Type::TEMPERATURE :
      _name   = "Temperature";
      _units  = "F";
      _scale  = 1.8;
      _offset = 32.0;
      break;
    case Type::HUMIDITY :
      _name   = "Humidity";
      _units  = "%";
      _scale  = 1.0;
      _offset = 0.0;
      break;
    case Type::PRESSURE :
      _name   = "Pressure";
      _units  = "in-Hg";
      _scale  = 0.02953 / 100.0;
      _offset = 0.0;
      break;
    default:
      _name   = "Unknown";
      _units  = "?";
      _scale  = 1.0;
      _offset = 0.0;
      break;
  }
}

bool Sensor::Data::update(float v) 
{
  if (!_blank) {
    if (std::isnan(v)){
      Serial.printf("failure to read %s\n", _name);
      return false;
    }
    set( _scale*v + _offset );
  }
  return true;
}

void  Sensor::Data::difference(Sensor::Data& s1, Sensor::Data& s2) 
{
    if (!_blank) 
    {
      set(s1.value() - s2.value());
    }
}

void    Sensor::Data::blank(void)
{
  _blank = true;
  _units = "";
}

void  Sensor::Data::println(const char* desc) {
  Serial.printf("%s %s %s\n", desc, str(), _units); 
}

/////////////////////////////////////////////////////////

bool BME280Sensor::init(uint8_t addr) 
{
  bool status = _sensor.begin(addr);
  if (!status) {
    Serial.printf("could not find a valid BME280 sensor using addr=%x\n",addr);
  }
  return status;
}

bool  BME280Sensor::read(void) 
{
  bool status = true;
  if (!_temperature.update(_sensor.readTemperature()))
    status = false;
  if (!_humidity.update(_sensor.readHumidity()))
    status = false;
  if (!_pressure.update(_sensor.readPressure()))
    status = false;
  return status;
}

///////////////////////////////////////////////////////////////////

bool  SHT31Sensor::init(uint8_t addr)
{
  bool status = _sensor.begin(addr);  // 0x44, or 0x45
  if (!status)
    Serial.println("Couldn't find SHT31");
  return status;  
}

bool  SHT31Sensor::read(void)
{
  bool status = true;
  if (!_temperature.update(_sensor.readTemperature() ))
    status = false;
  if (!_humidity.update(_sensor.readHumidity()))
    status = false;
  return status;
}

/////////////////////////////////////////////////////////

void  DeltaSensor::difference(Sensor& s1, Sensor& s2) {
  _temperature.difference( s1.temperature(), s2.temperature());
  _humidity.difference( s1.humidity(), s2.humidity());
}

//
/////////////////////////////////////////////////////////
//

Sensor::Sensor(Sensor::Type t)  :  _type(t), 
    _temperature(Data::Type::TEMPERATURE), 
    _humidity(Data::Type::HUMIDITY), 
    _pressure(Data::Type::PRESSURE) 
{
    switch(t) 
    {
        case Sensor::Type::BME280 : _name = "BME280"; _htmlname = _name; break;
        case Sensor::Type::DHT22 :  _name = "DHT22";  _htmlname = _name; break;
        case Sensor::Type::SHT31 :  _name = "SHT31";  _htmlname = _name; break;
        case Sensor::Type::DELTA :  _name = "Delta";  _htmlname = "&Delta;"; break;
        default: break;
    }
}

//
/////////////////////////////////////////////////////////
//
/*
template <typename T>
Readings<T>::Readings()
{
  _values.reserve(Readings<T>::MAX_SIZE);
}
*/

template <typename T>
void  Readings<T>::add(T t)
{
  int n = _values.size();
  if (n < Readings::MAX_SIZE)
  {
    _values.push_back( t );
  }
  else
  {
    // mover every element down, and put the new one at the end
    for(int i=0; i< n-1; i++)
      _values[i] = _values[i+1];  
    _values[n-1] = t;
  }
  
  _minmax = MinMax<T>(); // initialize each time to only get this graphs min/max
  for(int i=0; i< _values.size(); i++)
    _minmax.update( _values[i] );
}

/*
SensorGroup::SensorGroup() : 
  _temperatures(            Float(1000.0),Float(-1000)),
  _temperatureGlobalMinMax( Float(1000.0),Float(-1000)),
  _deltas(                  Float(1000.0),Float(-1000)),
  _deltaGlobalMinMax(       Float(1000.0),Float(-1000)),
  _times(1000.0, -1000)
{
}
*/

void  SensorGroup::init(void)
{
  Serial.println("starting sensors...");
  _bme280.init();
  _sht31.init();  
  _measurements = 0;
  _sensors.push_back(&_bme280);
  _sensors.push_back(&_sht31);
  _sensors.push_back(&_delta);
}

void  SensorGroup::read(void)
{
  int time = int(millis()/1000.0);
  _bme280.read();
  yield();
  _sht31.read();
  yield();
  _delta.difference(_bme280, _sht31);
  _measurements++;
  
  _times.add( time );
  _deltas.add( Float(_delta.temperature().value()));
  _temperatures.add( Float(_bme280.temperature().value()));

  // globals
  _deltaGlobalMinMax.update( Float(_delta.temperature().value()) );
  _temperatureGlobalMinMax.update( Float(_bme280.temperature().value()) );
 
  Serial.printf("\ntime=%d sec\n",int(millis()/1000.0));
  Serial.print("bme280 temperature=");Serial.println(_bme280.temperature().str());
  Serial.print(" sht31 temperature=");Serial.println(_sht31.temperature().str());
  Serial.print(" delta temperature=");Serial.println(_delta.temperature().str());
}

void  SensorGroup::getDisplayLines(Sensor::Data::Type type, char lines[3][64])
{
    char buffer[128];
    switch (type) 
    {
      case Sensor::Data::TEMPERATURE :
        sprintf(lines[0],"%s","Temperature (F)") ;
        sprintf(lines[1],    "bme280 = %s", _bme280.temperature().str());
        sprintf(lines[2],"     sht31 = %s", _sht31.temperature().str());
        break;
      case Sensor::Data::HUMIDITY :
        sprintf(lines[0],"%s","Humidity (%)") ;
        sprintf(lines[1],    "bme280 = %s", _bme280.humidity().str());
        sprintf(lines[2],"     sht31 = %s", _sht31.humidity().str());
        break;
    }
}

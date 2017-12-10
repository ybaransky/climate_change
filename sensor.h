#ifndef SENSOR_H
#define SENSOR_H

#include <utility>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SHT31.h>

class Float
{
  enum {DECIMALS=1, STR_SIZE=16};
  public:
    Float(float f=0.0);
    Float&  operator=(const Float& rhs);
    friend bool operator<(const Float& lhs, const Float& rhs);

    void          set(float f, int decimals=Float::DECIMALS);
    const char*   str(void) const  {return _str;}
    const char*   str2(void) const {return _str2;}

    float         value(void) const   { return _float; }
    void          value(float f)      { set(f); }

  private:
    float   _float;
    char    _str[STR_SIZE];
    char    _str2[STR_SIZE];
};

class Sensor 
{
  public :

  class Data : public Float
  {
    public :
      enum Type {TEMPERATURE, HUMIDITY, PRESSURE};

      Data(Data::Type t);
         
      void    println(const char* desc);                
      bool    update(float v);
      void    difference(Sensor::Data&, Sensor::Data&);
      void    value2string(int decimals=2);
      void    blank();
      
      const char*   name() const       {return _name;}
          
    private :
      float       _scale, _offset;
      Float       _float;
      const char* _name;
      const char* _units;
      Data::Type  _type;
      bool        _blank;
  };    
     
  public:
    enum Type {BME280, DHT22, SHT31, DELTA};

    Sensor(Sensor::Type t);
    virtual bool  init(uint8_t addr)=0;
    virtual bool  read(void)=0;

    const Sensor::Type  type(bool html) const { return _type;}
    const char*         name()      const { return _name; }
    const char*         htmlname()  const { return _htmlname; }

    Sensor::Data& temperature(void) {return _temperature;}
    Sensor::Data& humidity (void)   {return _humidity;}
    Sensor::Data& pressure(void)    {return _pressure;}
  
  protected:
    const char*   _name;
    const char*   _htmlname;
    Sensor::Type  _type;
    Sensor::Data  _temperature;
    Sensor::Data  _humidity;
    Sensor::Data  _pressure;
};

class BME280Sensor : public Sensor {
  public : 
    BME280Sensor(void) : Sensor(Sensor::Type::BME280) {}
    virtual bool  init(uint8_t addr=0x76);
    virtual bool  read(void);
  private :
    Adafruit_BME280 _sensor;
};

class SHT31Sensor : public Sensor {
  public : 
    SHT31Sensor(void) : Sensor(Sensor::Type::SHT31) {}
    virtual bool  init(uint8_t addr=0x44);
    virtual bool  read(void);
  private :
    Adafruit_SHT31 _sensor;  
};

class DeltaSensor : public Sensor {
  public : 
    DeltaSensor(void) : Sensor(Sensor::Type::DELTA) {}
    bool  init(uint8_t addr) {}
    bool  read(void) {}
    void  difference(Sensor&, Sensor&);
};

typedef std::vector<Sensor*>  Sensors;
typedef Sensors::iterator   SensorsIter;

template <typename T>
class MinMax
{
    public:
      MinMax(T tmin=T(1000), T tmax=T(-1000)) : _tmin(tmin), _tmax(tmax) {}
      
      const T&  min(void) const {return _tmin;}
      const T&  max(void) const {return _tmax;}
      void      min(const T t)  {_tmin = t;}
      void      max(const T t) {  _tmax = t;}

      void      update(T t)
                {
                  if (t < _tmin) _tmin = t;
                  if (_tmax < t) _tmax = t;
                }
    private:
      T         _tmin;
      T         _tmax;
};

template <typename T>
class Readings
{
  static const int MAX_SIZE = 103;
  
  public :
    Readings() { _values.reserve(MAX_SIZE); }
    void                        add(T);
    const std::vector<T>&       values() const { return _values; }
    const MinMax<T>&            minmax() const { return _minmax; }
  
  private:
    std::vector<T>              _values;
    MinMax<T>                   _minmax;
};

typedef std::vector<int>        Times;
typedef MinMax<int>             TimeMinMax;
typedef Readings<int>           TimeReadings;

typedef std::vector<Float>      Floats;
typedef MinMax<Float>           FloatMinMax;
typedef Readings<Float>         FloatReadings;

class SensorGroup
{
public:
  SensorGroup() {};
  void            init(void);
  void            read(void);
  void            getDisplayLines(Sensor::Data::Type, char lines[3][64]);

  int             measurements(void)  {return _measurements;}
  BME280Sensor&   bme280(void)        {return _bme280;}
  SHT31Sensor&    sht31(void)         {return _sht31;}
  DeltaSensor     delta(void)         {return _delta;}

  Sensors&        sensors()           {return _sensors;}
  FloatReadings&  temperatures()      {return _temperatures; }
  FloatReadings&  deltas()            {return _deltas;    }
  TimeReadings&   times()             {return _times;    } 

  FloatMinMax&    temperatureGlobalMinMax()  { return _temperatureGlobalMinMax;}
  FloatMinMax&    deltaGlobalMinMax()        { return _deltaGlobalMinMax;}
  
private : 
  BME280Sensor    _bme280;
  SHT31Sensor     _sht31;
  DeltaSensor     _delta;
  int             _measurements;

  Sensors         _sensors;
  FloatReadings   _temperatures;
  FloatReadings   _deltas;
  TimeReadings    _times;
  FloatMinMax     _deltaGlobalMinMax;
  FloatMinMax     _temperatureGlobalMinMax;
};

extern  SensorGroup gSensorGroup;

#endif

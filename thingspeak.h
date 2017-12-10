#ifndef THINGSPEAK_H
#define THINGSPEAK_H

class Thingspeak
{
  static const int  HTTP_PORT=80;

  public:
      Thingspeak();

      bool  update(void);
      int   updates(void) const { return _updates;}

  private:
      int   _updates;
};

extern Thingspeak   gThingspeak;

#endif

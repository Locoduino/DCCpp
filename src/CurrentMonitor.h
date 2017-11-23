/**********************************************************************

CurrentMonitor.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef CurrentMonitor_h
#define CurrentMonitor_h

//#include "Arduino.h"

#define  CURRENT_SAMPLE_SMOOTHING   0.01

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO
	#define  CURRENT_SAMPLE_TIME        10
#else                                         // Configuration for MEGA    
	#define  CURRENT_SAMPLE_TIME        1
#endif

struct CurrentMonitor{
  static long int sampleTime;
  int pin;
  float currentSampleMax;
  float current;
  const char *msg;
  void begin(int, const char *, float = 300);
  static boolean checkTime();
  void check();
};

#endif


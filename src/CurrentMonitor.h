/**********************************************************************

CurrentMonitor.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef CurrentMonitor_h
#define CurrentMonitor_h

/** Factor to smooth the result...*/
#define  CURRENT_SAMPLE_SMOOTHING   0.01

/** Time between two measurements.
@remark Millis() uses TIMER-0.  For UNO, we change the scale on Timer-0.  For MEGA we do not.  This means CURRENT_SAMPLE_TIME is different for UNO then MEGA.
*/
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO
	#define  CURRENT_SAMPLE_TIME        10
#else                                         // Configuration for MEGA    
	#define  CURRENT_SAMPLE_TIME        1
#endif

/** This structure/class describes a current monitor.*/

struct CurrentMonitor{
  static long int sampleTime; /**< time elapsed since last measurement. This delay is common to all monitors. */
	int pin;	/**< Attached check pin.*/
	int signalPin;	/**< signal pin.*/
  float currentSampleMax;	/**< Value of the maximum current accepted without problem.*/
  float current; /**< Value of the last measured current.*/
  const char *msg;	/**< Message to send when excessive current is measured.*/
  /** begin function.
	@param pin	Attached pin. UNEFINED_PIN to inactivate this monitor.
	@param inSignalPin	Pin to set LOW if a shortcut is detectexd.
  @param msg	Message to send to console when a smoothed current value greater than maximum is detected.
  @param inSampleMax	Maximum value of the current. Default is 300.
  */
  void begin(int pin, int inSignalPin, const char *msg, float inSampleMax = 300);
  /** Checks if sufficient time has elapsed since last update. Common to all monitors.
  */
  static boolean checkTime();
  /** Checks the current on this monitor.
  */
  void check();
};

#endif

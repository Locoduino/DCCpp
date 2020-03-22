/**********************************************************************

CurrentMonitor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"

#include "DCCpp_Uno.h"
#include "CurrentMonitor.h"
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

void CurrentMonitor::begin(int pin, int inSignalPin, const char *msg, float inSampleMax)
{
	this->pin = pin;
	this->signalPin = inSignalPin;
	this->msg = msg;
	this->current = 0;
	this->currentSampleMax = inSampleMax;
} // CurrentMonitor::begin
  
boolean CurrentMonitor::checkTime()
{
	if(millis( ) - sampleTime < CURRENT_SAMPLE_TIME)            // no need to check current yet
		return(false);
	sampleTime = millis();                                   // note millis() uses TIMER-0.  For UNO, we change the scale on Timer-0.  For MEGA we do not.  This means CURENT_SAMPLE_TIME is different for UNO then MEGA
	return(true);  
} // CurrentMonitor::checkTime
  
void CurrentMonitor::check()
{
	if (this->pin == UNDEFINED_PIN || this->signalPin == UNDEFINED_PIN)
		return;

	this->current = (float)(analogRead(this->pin) * CURRENT_SAMPLE_SMOOTHING + this->current * (1.0 - CURRENT_SAMPLE_SMOOTHING));      // compute new exponentially-smoothed current

	// current overload and Signal is on
	if (this->current > this->currentSampleMax && digitalRead(this->signalPin) == HIGH)
	{
		digitalWrite(this->signalPin, LOW);
		DCCPP_INTERFACE.print(this->msg);                                     // print corresponding error message
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
	}
} // CurrentMonitor::check  

long int CurrentMonitor::sampleTime=0;

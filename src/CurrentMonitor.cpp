/**********************************************************************

CurrentMonitor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp_Uno.h"
#include "CurrentMonitor.h"

///////////////////////////////////////////////////////////////////////////////

void CurrentMonitor::begin(int pin, const char *msg)
{
	this->pin = pin;
	this->msg = msg;
	this->current = 0;
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
	if (this->pin == 255)
		return;
	this->current = (float) (analogRead(this->pin) * CURRENT_SAMPLE_SMOOTHING + this->current * (1.0-CURRENT_SAMPLE_SMOOTHING));      // compute new exponentially-smoothed current
	int signalPin = DCCppConfig::SignalEnablePinProg;
	if (signalPin == 255)
		signalPin = DCCppConfig::SignalEnablePinMain;

	// current overload and Prog Signal is on (or could have checked Main Signal, since both are always on or off together)
	if (this->current > CURRENT_SAMPLE_MAX && digitalRead(signalPin) == HIGH)
	{
		if (DCCppConfig::SignalEnablePinProg != 255)
			digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);		// disable both Motor Shield Channels
		if (DCCppConfig::SignalEnablePinMain != 255)
			digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);        // regardless of which caused current overload
		INTERFACE.print(this->msg);                                     // print corresponding error message
	}
} // CurrentMonitor::check  

long int CurrentMonitor::sampleTime=0;


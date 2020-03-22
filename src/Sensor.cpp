/**********************************************************************

Sensor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"

#include "Sensor.h"
#ifdef USE_SENSOR

#ifdef VISUALSTUDIO
#include "string.h"
#endif
#include "DCCpp_Uno.h"
#include "EEStore.h"
#ifdef USE_EEPROM
#include "EEPROM.h"
#endif
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

void Sensor::begin(int snum, int pin, int pullUp) {
#if defined(USE_EEPROM)	&& defined(DCCPP_DEBUG_MODE)
	if (strncmp(EEStore::data.id, EESTORE_ID, sizeof(EESTORE_ID)) != 0) {    // check to see that eeStore contains valid DCC++ ID
		DCCPP_INTERFACE.println(F("Sensor::begin() must be called BEFORE DCCpp.begin() !"));
	}
#endif

	if (firstSensor == NULL) {
		firstSensor = this;
	}
	else if (get(snum) == NULL) {
		Sensor *tt = firstSensor;
		while (tt->nextSensor != NULL)
			tt = tt->nextSensor;
		tt->nextSensor = this;
	}

	this->set(snum, pin, pullUp);

#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<O>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::set(int snum, int pin, int pullUp) {
	this->data.snum = snum;
	this->data.pin = pin;
	this->data.pullUp = (pullUp == 0 ? LOW : HIGH);
	this->active = false;
	this->signal = 1;
#ifdef VISUALSTUDIO
	dontCheckNextPinAccess = true;
#endif
	digitalWrite(pin, pullUp);   // don't use Arduino's internal pull-up resistors for external infrared sensors --- each sensor must have its own 1K external pull-up resistor
#ifdef VISUALSTUDIO
	dontCheckNextPinAccess = true;
#endif
	pinMode(pin, INPUT);         // force mode to input
}

///////////////////////////////////////////////////////////////////////////////

Sensor* Sensor::get(int n) {
	Sensor *tt;
	for (tt = firstSensor; tt != NULL && tt->data.snum != n; tt = tt->nextSensor);
	return(tt);
}
///////////////////////////////////////////////////////////////////////////////

void Sensor::remove(int n) {
	Sensor *tt, *pp;

	for (tt = firstSensor, pp =  NULL; tt != NULL && tt->data.snum != n; pp = tt, tt = tt->nextSensor);

	if (tt == NULL) {
#ifdef USE_TEXTCOMMAND
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
#endif
		return;
	}

	if (tt == firstSensor)
		firstSensor = tt->nextSensor;
	else
		pp->nextSensor = tt->nextSensor;

	free(tt);

#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<O>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

int Sensor::count() {
	int count = 0;
	Sensor *tt;
	for (tt = firstSensor; tt != NULL; tt = tt->nextSensor)
		count++;
	return count;
}

///////////////////////////////////////////////////////////////////////////////
  
void Sensor::check(){
  Sensor *tt;

  for(tt = firstSensor; tt != NULL; tt = tt->nextSensor){
    tt->signal = (float)(tt->signal * (1.0 - SENSOR_DECAY) + digitalRead(tt->data.pin) * SENSOR_DECAY);
    
    if(!tt->active && tt->signal<0.5){
      tt->active=true;
	  DCCPP_INTERFACE.print("<Q");
      DCCPP_INTERFACE.print(tt->data.snum);
      DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
	  DCCPP_INTERFACE.println("");
#endif
	} else if(tt->active && tt->signal>0.9){
      tt->active=false;
	  DCCPP_INTERFACE.print("<q");
      DCCPP_INTERFACE.print(tt->data.snum);
      DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
	  DCCPP_INTERFACE.println("");
#endif
	}
  } // loop over all sensors
    
} // Sensor::check

#ifdef DCCPP_PRINT_DCCPP
  ///////////////////////////////////////////////////////////////////////////////

void Sensor::show() {
	Sensor *tt;

	if (firstSensor == NULL) {
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
		return;
	}

	for (tt = firstSensor; tt != NULL; tt = tt->nextSensor) {
		DCCPP_INTERFACE.print("<Q");
		DCCPP_INTERFACE.print(tt->data.snum);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.pin);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.pullUp);
		DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::status() {
	Sensor *tt;

	if (firstSensor == NULL) {
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
		return;
	}

	for (tt = firstSensor; tt != NULL; tt = tt->nextSensor) {
		DCCPP_INTERFACE.print(tt->active ? "<Q" : "<q");
		DCCPP_INTERFACE.print(tt->data.snum);
		DCCPP_INTERFACE.print(">");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
	}
}

#endif

#ifdef USE_EEPROM
///////////////////////////////////////////////////////////////////////////////

void Sensor::load() {
	struct SensorData data;
	Sensor *tt;

	for (int i = 0; i<EEStore::data.nSensors; i++) {
#ifdef VISUALSTUDIO
		EEPROM.get(EEStore::pointer(), (void *)&(data), sizeof(SensorData));	// ArduiEmulator version...
#else
		EEPROM.get(EEStore::pointer(), data);
#endif
#if defined(USE_TEXTCOMMAND)
		tt = create(data.snum, data.pin, data.pullUp);
#else
		tt = get(data.snum);
#ifdef DCCPP_DEBUG_MODE
		if (tt == NULL)
			DCCPP_INTERFACE.println(F("Sensor::begin() must be called BEFORE Sensor::load() !"));
		else
#endif
			tt->set(data.snum, data.pin, data.pullUp);
#endif
		EEStore::advance(sizeof(tt->data));
	}
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::store() {
	Sensor *tt;

	tt = firstSensor;
	EEStore::data.nSensors = 0;

	while (tt != NULL) {
#ifdef VISUALSTUDIO
		EEPROM.put(EEStore::pointer(), (void *)&(tt->data), sizeof(SensorData));	// ArduiEmulator version...
#else
		EEPROM.put(EEStore::pointer(), tt->data);
#endif
		EEStore::advance(sizeof(tt->data));
		tt = tt->nextSensor;
		EEStore::data.nSensors++;
	}
}
#endif

#if defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

bool Sensor::parse(char *c) {
	int n, s, m;
	//  Sensor *t;

	switch (sscanf(c, "%d %d %d", &n, &s, &m)) {

	case 3:                     // argument is string with id number of sensor followed by a pin number and pullUp indicator (0=LOW/1=HIGH)
		create(n, s, m);
		return true;

	case 1:                     // argument is a string with id number only
		remove(n);
		return true;

#ifdef DCCPP_PRINT_DCCPP
	case -1:                    // no arguments
		show();
		return true;
#endif
#ifdef USE_TEXTCOMMAND
	case 2:                     // invalid number of arguments
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
		return true;
#endif
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

Sensor *Sensor::create(int snum, int pin, int pullUp) {
	Sensor *tt = new Sensor();

	if (tt == NULL) {       // problem allocating memory
#ifdef USE_TEXTCOMMAND
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
#endif
		return(tt);
	}

	tt->begin(snum, pin, pullUp);

	return(tt);
}

#endif

///////////////////////////////////////////////////////////////////////////////

Sensor *Sensor::firstSensor=NULL;

#endif

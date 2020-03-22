/**********************************************************************

Outputs.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
#include "Arduino.h"

#include "DCCpp.h"

#ifdef USE_OUTPUT
#include "Outputs.h"

#ifdef VISUALSTUDIO
#include "string.h"
#endif
#include "TextCommand.h"
#include "DCCpp_Uno.h"
#include "EEStore.h"
#ifdef USE_EEPROM
#include "EEPROM.h"
#endif
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

void Output::begin(int id, int pin, int iFlag) {
#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
#if defined(USE_EEPROM)	&& defined(DCCPP_DEBUG_MODE)
	if (strncmp(EEStore::data.id, EESTORE_ID, sizeof(EESTORE_ID)) != 0) {    // check to see that eeStore contains valid DCC++ ID
		DCCPP_INTERFACE.println(F("Output::begin() must be called BEFORE DCCpp.begin() !"));
	}
#endif
	if (firstOutput == NULL) {
		firstOutput = this;
	}
	else if ((get(id)) == NULL) {
		Output *tt = firstOutput;
		while (tt->nextOutput != NULL)
			tt = tt->nextOutput;
		tt->nextOutput = this;
	}
#endif

	this->set(id, pin, iFlag);

#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<O>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

void Output::set(int id, int pin, int iFlag) {
	this->data.id = id;
	this->data.pin = pin;
	this->data.iFlag = iFlag;
	this->data.oStatus = 0;

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag  
	this->data.oStatus = bitRead(this->data.iFlag, 1) ? bitRead(this->data.iFlag, 2) : 0;
#ifdef VISUALSTUDIO
	dontCheckNextPinAccess = true;
#endif
	digitalWrite(this->data.pin, this->data.oStatus ^ bitRead(this->data.iFlag, 0));
#ifdef VISUALSTUDIO
	dontCheckNextPinAccess = true;
#endif
	pinMode(this->data.pin, OUTPUT);
}

///////////////////////////////////////////////////////////////////////////////

void Output::activate(int s){
  data.oStatus=(s>0);                                               // if s>0, set status to active, else inactive
  digitalWrite(data.pin,data.oStatus ^ bitRead(data.iFlag,0));      // set state of output pin to HIGH or LOW depending on whether bit zero of iFlag is set to 0 (ACTIVE=HIGH) or 1 (ACTIVE=LOW)
#ifdef USE_EEPROM
  if(num>0)
#ifdef VISUALSTUDIO
	  EEPROM.put(num, (void *)&data.oStatus, 1);
#else
	  EEPROM.put(num, data.oStatus);
#endif
#endif
#ifdef USE_TEXTCOMMAND
  DCCPP_INTERFACE.print("<Y");
  DCCPP_INTERFACE.print(data.id);
  if(data.oStatus==0)
    DCCPP_INTERFACE.print(" 0>");
  else
    DCCPP_INTERFACE.print(" 1>"); 
#if !defined(USE_ETHERNET)
  DCCPP_INTERFACE.println("");
#endif
#endif
}

#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

Output* Output::get(int n){
  Output *tt;
  for(tt=firstOutput;tt!=NULL && tt->data.id!=n;tt=tt->nextOutput);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void Output::remove(int n){
  Output *tt,*pp;
  
  for(tt=firstOutput, pp = NULL;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextOutput);

  if(tt==NULL){
#ifdef USE_TEXTCOMMAND
	DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
#endif
#endif
    return;
  }
  
  if(tt==firstOutput)
    firstOutput=tt->nextOutput;
  else
    pp->nextOutput=tt->nextOutput;

  free(tt);

#ifdef USE_TEXTCOMMAND
  DCCPP_INTERFACE.print("<O>");
#if !defined(USE_ETHERNET)
  DCCPP_INTERFACE.println("");
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////

int Output::count() {
	int count = 0;
	Output *tt;
	for (tt = firstOutput; tt != NULL; tt = tt->nextOutput)
		count++;
	return count;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_EEPROM
void Output::load() {
	struct OutputData data;
	Output *tt;

	for (int i = 0; i<EEStore::data.nOutputs; i++) {
#ifdef VISUALSTUDIO
		EEPROM.get(EEStore::pointer(), (void *)&data, sizeof(OutputData));	// ArduiEmulator version...
#else
		EEPROM.get(EEStore::pointer(), data);
#endif
#if defined(USE_TEXTCOMMAND)
		tt = create(data.id, data.pin, data.iFlag);
#else
		tt = get(data.id);
#ifdef DCCPP_DEBUG_MODE
		if (tt == NULL)
			DCCPP_INTERFACE.println(F("Output::begin() must be called BEFORE Output::load() !"));
		else
#endif
			tt->set(data.id, data.pin, data.iFlag);
#endif

		tt->data.oStatus = bitRead(tt->data.iFlag, 1) ? bitRead(tt->data.iFlag, 2) : data.oStatus;      // restore status to EEPROM value is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
#ifdef VISUALSTUDIO
		dontCheckNextPinAccess = true;
#endif
		digitalWrite(tt->data.pin, tt->data.oStatus ^ bitRead(tt->data.iFlag, 0));
#ifdef VISUALSTUDIO
		dontCheckNextPinAccess = true;
#endif
		pinMode(tt->data.pin, OUTPUT);
		tt->num = EEStore::pointer();
		EEStore::advance(sizeof(tt->data));
	}
}

///////////////////////////////////////////////////////////////////////////////

void Output::store() {
	Output *tt;

	tt = firstOutput;
	EEStore::data.nOutputs = 0;

	while (tt != NULL) {
		tt->num = EEStore::pointer();
#ifdef VISUALSTUDIO
		EEPROM.put(EEStore::pointer(), (void *)&(tt->data), sizeof(OutputData));	// ArduiEmulator version...
#else
		EEPROM.put(EEStore::pointer(), tt->data);
#endif
		EEStore::advance(sizeof(tt->data));
		tt = tt->nextOutput;
		EEStore::data.nOutputs++;
	}
}
#endif

#endif

#if defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

bool Output::parse(char *c){
  int n,s,m;
  Output *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 2:                     // argument is string with id number of output followed by zero (LOW) or one (HIGH)
      t=get(n);
      if(t!=NULL)
        t->activate(s);
			else
			{
			  DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
				DCCPP_INTERFACE.println("");
#endif
			}
			return true;

    case 3:                     // argument is string with id number of output followed by a pin number and invert flag
      create(n,s,m);
			return true;

    case 1:                     // argument is a string with id number only
      remove(n);
			return true;
    
#ifdef DCCPP_PRINT_DCCPP
	case -1:                    // no arguments
      show();                  // verbose show
#endif
			return true;
  }
	return false;
}

///////////////////////////////////////////////////////////////////////////////

Output *Output::create(int id, int pin, int iFlag){
	Output *tt = new Output();

	if (tt == NULL) {       // problem allocating memory
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
		return(tt);
	}

	tt->begin(id, pin, iFlag);
  
	return(tt);
}

#endif

#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
#ifdef DCCPP_PRINT_DCCPP

///////////////////////////////////////////////////////////////////////////////

void Output::show() {
	Output *tt;

	if (firstOutput == NULL) {
		DCCPP_INTERFACE.print("<X>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
		return;
	}

	for (tt = firstOutput; tt != NULL; tt = tt->nextOutput) {
		DCCPP_INTERFACE.print("<Y");
		DCCPP_INTERFACE.print(tt->data.id);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.pin);
		DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(tt->data.iFlag);

		if (tt->data.oStatus == 0)
			DCCPP_INTERFACE.print(" 0>");
		else
			DCCPP_INTERFACE.print(" 1>");
#if !defined(USE_ETHERNET)
		DCCPP_INTERFACE.println("");
#endif
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////

Output *Output::firstOutput=NULL;

#endif

#endif //USE_OUTPUT

/**********************************************************************

Outputs.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
#include "Outputs.h"

#ifdef USE_TURNOUT
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
#ifdef DCCPP_DEBUG_MODE
	if (EEStore::eeStore != NULL)
	{
		INTERFACE.println(F("Output::begin() must be called BEFORE DCCpp.begin() !"));
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

#ifdef DCCPP_DEBUG_MODE
	INTERFACE.println("<O>");
#endif
}

///////////////////////////////////////////////////////////////////////////////

void Output::set(int id, int pin, int iFlag) {
	this->data.id = id;
	this->data.pin = pin;
	this->data.iFlag = iFlag;
	this->data.oStatus = 0;

	pinMode(this->data.pin, OUTPUT);

	// sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag  
	this->data.oStatus = bitRead(this->data.iFlag, 1) ? bitRead(this->data.iFlag, 2) : 0;
	digitalWrite(this->data.pin, this->data.oStatus ^ bitRead(this->data.iFlag, 0));
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
#ifdef DCCPP_DEBUG_MODE
  INTERFACE.print("<Y");
  INTERFACE.print(data.id);
  if(data.oStatus==0)
    INTERFACE.println(" 0>");
  else
    INTERFACE.println(" 1>"); 
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
  
  for(tt=firstOutput;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextOutput);

  if(tt==NULL){
#ifdef DCCPP_DEBUG_MODE
	INTERFACE.println("<X>");
#endif
    return;
  }
  
  if(tt==firstOutput)
    firstOutput=tt->nextOutput;
  else
    pp->nextOutput=tt->nextOutput;

  free(tt);

#ifdef DCCPP_DEBUG_MODE
  INTERFACE.println("<O>");
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

	for (int i = 0; i<EEStore::eeStore->data.nOutputs; i++) {
#ifdef VISUALSTUDIO
		EEPROM.get(EEStore::pointer(), (void *)&data, sizeof(OutputData));	// ArduiEmulator version...
#else
		EEPROM.get(EEStore::pointer(), data);
#endif
		tt = get(data.id);
		tt->set(data.id, data.pin, data.iFlag);
		tt->data.oStatus = bitRead(tt->data.iFlag, 1) ? bitRead(tt->data.iFlag, 2) : data.oStatus;      // restore status to EEPROM value is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
		digitalWrite(tt->data.pin, tt->data.oStatus ^ bitRead(tt->data.iFlag, 0));
		pinMode(tt->data.pin, OUTPUT);
		tt->num = EEStore::pointer();
		EEStore::advance(sizeof(tt->data));
	}
}

///////////////////////////////////////////////////////////////////////////////

void Output::store() {
	Output *tt;

	tt = firstOutput;
	EEStore::eeStore->data.nOutputs = 0;

	while (tt != NULL) {
		tt->num = EEStore::pointer();
#ifdef VISUALSTUDIO
		EEPROM.put(EEStore::pointer(), (void *)&(tt->data), sizeof(OutputData));	// ArduiEmulator version...
#else
		EEPROM.put(EEStore::pointer(), tt->data);
#endif
		EEStore::advance(sizeof(tt->data));
		tt = tt->nextOutput;
		EEStore::eeStore->data.nOutputs++;
	}
}
#endif

#endif

#if defined(USE_TEXTCOMMAND)
///////////////////////////////////////////////////////////////////////////////

void Output::parse(char *c){
  int n,s,m;
  Output *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 2:                     // argument is string with id number of output followed by zero (LOW) or one (HIGH)
      t=get(n);
      if(t!=NULL)
        t->activate(s);
#ifdef DCCPP_PRINT_DCCPP
	  else
		  INTERFACE.println("<X>");
#endif
      break;

    case 3:                     // argument is string with id number of output followed by a pin number and invert flag
      create(n,s,m,1);
    break;

    case 1:                     // argument is a string with id number only
      remove(n);
    break;
    
#ifdef DCCPP_PRINT_DCCPP
	case -1:                    // no arguments
      show(1);                  // verbose show
#endif
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

Output *Output::create(int id, int pin, int iFlag){
	Output *tt = new Output();

	if (tt == NULL) {       // problem allocating memory
#ifdef DCCPP_PRINT_DCCPP
		INTERFACE.println("<X>");
#endif
		return(tt);
	}

	tt->begin(id, pin, iFlag);
  
	return(tt);
}

#endif USE_TEXTCOMMAND

#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
#ifdef DCCPP_PRINT_DCCPP

///////////////////////////////////////////////////////////////////////////////

void Output::show() {
	Output *tt;

	if (firstOutput == NULL) {
		INTERFACE.print("<X>");
		return;
	}

	for (tt = firstOutput; tt != NULL; tt = tt->nextOutput) {
		INTERFACE.print("<Y");
		INTERFACE.print(tt->data.id);
		INTERFACE.print(" ");
		INTERFACE.print(tt->data.pin);
		INTERFACE.print(" ");
		INTERFACE.print(tt->data.iFlag);

		if (tt->data.oStatus == 0)
			INTERFACE.println(" 0>");
		else
			INTERFACE.println(" 1>");
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////

Output *Output::firstOutput=NULL;

#endif

#endif USE_OUTPUT

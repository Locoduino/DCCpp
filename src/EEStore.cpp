/**********************************************************************

EEStore.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Arduino.h"
#ifdef ARDUINO_ARCH_AVR

#include "EEStore.h"

#ifdef USE_EEPROM
#ifdef VISUALSTUDIO
#include "string.h"
#endif
#include "DCCpp_Uno.h"
#include "Turnout.h"
#include "Sensor.h"
#include "Outputs.h"
#include "EEPROM.h"

///////////////////////////////////////////////////////////////////////////////

void EEStore::init(){

  
  //eeStore=(EEStore *)calloc(1,sizeof(EEStore));

  //EEPROM.get(0,eeStore->data);                                       // get eeStore data 
#ifdef VISUALSTUDIO
  EEPROM.get(0, (void *)&data, sizeof(EEStoreData));
#else
  EEPROM.get(0, data);
#endif
  
  if(strncmp(data.id,EESTORE_ID,sizeof(EESTORE_ID))!=0){    // check to see that eeStore contains valid DCC++ ID
    sprintf(data.id,EESTORE_ID);                           // if not, create blank eeStore structure (no turnouts, no sensors) and save it back to EEPROM
#ifdef USE_TURNOUT
	data.nTurnouts=0;
#endif
#ifdef USE_SENSOR
	data.nSensors=0;
#endif
#ifdef USE_OUTPUT
	data.nOutputs=0;
#endif
#ifdef VISUALSTUDIO
	EEPROM.put(0, (void *)&data, sizeof(EEStoreData));
#else
	EEPROM.put(0, data);
#endif
  }
  
  reset();            // set memory pointer to first free EEPROM space
#ifdef USE_TURNOUT
  Turnout::load();    // load turnout definitions
#endif
#ifdef USE_SENSOR
  Sensor::load();     // load sensor definitions
#endif
#ifdef USE_OUTPUT
  Output::load();     // load output definitions
#endif  
}

///////////////////////////////////////////////////////////////////////////////

void EEStore::clear(){
    
  sprintf(data.id,EESTORE_ID);                           // create blank eeStore structure (no turnouts, no sensors) and save it back to EEPROM
#ifdef USE_TURNOUT
  data.nTurnouts=0;
#endif
#ifdef USE_SENSOR
  data.nSensors=0;
#endif
#ifdef USE_OUTPUT
  data.nOutputs=0;
#endif
#ifdef VISUALSTUDIO
  EEPROM.put(0, (void *)&data, sizeof(EEStoreData));
#else
  EEPROM.put(0, data);
#endif

}

///////////////////////////////////////////////////////////////////////////////

void EEStore::store() {
	reset();
#ifdef USE_TURNOUT
	Turnout::store();
#endif
#ifdef USE_SENSOR
	Sensor::store();
#endif
#ifdef USE_OUTPUT
	Output::store();
#endif
#ifdef VISUALSTUDIO
	EEPROM.put(0, (void *)&data, sizeof(EEStoreData));
#else
	EEPROM.put(0, data);
#endif
}

///////////////////////////////////////////////////////////////////////////////

bool EEStore::needsRefreshing() {
#ifdef USE_TURNOUT
	if (data.nTurnouts != Turnout::count())
		return true;
#endif
#ifdef USE_SENSOR
	if (data.nSensors != Sensor::count())
		return true;
#endif
#ifdef USE_OUTPUT
	if (data.nOutputs!= Output::count())
		return true;
#endif
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void EEStore::advance(int n){
  eeAddress+=n;
}

///////////////////////////////////////////////////////////////////////////////

void EEStore::reset(){
  eeAddress=sizeof(EEStoreData);
}
///////////////////////////////////////////////////////////////////////////////

int EEStore::pointer(){
  return(eeAddress);
}
///////////////////////////////////////////////////////////////////////////////

//EEStore *EEStore::eeStore=NULL;
EEStoreData EEStore::data;

int EEStore::eeAddress=0;

#endif
#endif

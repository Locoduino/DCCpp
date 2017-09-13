/**********************************************************************

Sensor.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Sensor.h"
#ifdef USE_SENSOR
#include "DCCpp_Uno.h"
#include "EEStore.h"
#ifdef USE_EEPROM
#include "EEPROM.h"
#endif
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////
  
void Sensor::check(){    
  Sensor *tt;

  for(tt=firstSensor;tt!=NULL;tt=tt->nextSensor){
    tt->signal=(float)(tt->signal*(1.0-SENSOR_DECAY)+digitalRead(tt->data.pin)*SENSOR_DECAY);
    
    if(!tt->active && tt->signal<0.5){
      tt->active=true;
      INTERFACE.print("<Q");
      INTERFACE.print(tt->data.snum);
      INTERFACE.print(">");
    } else if(tt->active && tt->signal>0.9){
      tt->active=false;
      INTERFACE.print("<q");
      INTERFACE.print(tt->data.snum);
      INTERFACE.print(">");
    }
  } // loop over all sensors
    
} // Sensor::check

///////////////////////////////////////////////////////////////////////////////

Sensor *Sensor::create(int snum, int pin, int pullUp, int v){
  Sensor *tt;
  
  if(firstSensor==NULL){
    firstSensor=(Sensor *)calloc(1,sizeof(Sensor));
    tt=firstSensor;
  } else if((tt=get(snum))==NULL){
    tt=firstSensor;
    while(tt->nextSensor!=NULL)
      tt=tt->nextSensor;
    tt->nextSensor=(Sensor *)calloc(1,sizeof(Sensor));
    tt=tt->nextSensor;
  }

  if(tt==NULL){       // problem allocating memory
    if(v==1)
      INTERFACE.print("<X>");
    return(tt);
  }
  
  tt->data.snum=snum;
  tt->data.pin=pin;
  tt->data.pullUp=(pullUp==0?LOW:HIGH);
  tt->active=false;
  tt->signal=1;
  pinMode(pin,INPUT);         // set mode to input
  digitalWrite(pin,pullUp);   // don't use Arduino's internal pull-up resistors for external infrared sensors --- each sensor must have its own 1K external pull-up resistor

  if(v==1)
    INTERFACE.print("<O>");
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

Sensor* Sensor::get(int n){
  Sensor *tt;
  for(tt=firstSensor;tt!=NULL && tt->data.snum!=n;tt=tt->nextSensor);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void Sensor::remove(int n){
  Sensor *tt,*pp;
  
  for(tt=firstSensor;tt!=NULL && tt->data.snum!=n;pp=tt,tt=tt->nextSensor);

  if(tt==NULL){
    INTERFACE.print("<X>");
    return;
  }
  
  if(tt==firstSensor)
    firstSensor=tt->nextSensor;
  else
    pp->nextSensor=tt->nextSensor;

  free(tt);

  INTERFACE.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::show(){
  Sensor *tt;

  if(firstSensor==NULL){
    INTERFACE.print("<X>");
    return;
  }
    
  for(tt=firstSensor;tt!=NULL;tt=tt->nextSensor){
    INTERFACE.print("<Q");
    INTERFACE.print(tt->data.snum);
    INTERFACE.print(" ");
    INTERFACE.print(tt->data.pin);
    INTERFACE.print(" ");
    INTERFACE.print(tt->data.pullUp);
    INTERFACE.println(">");
  }
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::status(){
  Sensor *tt;

  if(firstSensor==NULL){
    INTERFACE.print("<X>");
    return;
  }
    
  for(tt=firstSensor;tt!=NULL;tt=tt->nextSensor){
    INTERFACE.print(tt->active?"<Q":"<q");
    INTERFACE.print(tt->data.snum);
    INTERFACE.print(">");
  }
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::parse(char *c){
  int n,s,m;
//  Sensor *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 3:                     // argument is string with id number of sensor followed by a pin number and pullUp indicator (0=LOW/1=HIGH)
      create(n,s,m,1);
    break;

    case 1:                     // argument is a string with id number only
      remove(n);
    break;
    
    case -1:                    // no arguments
      show();
    break;

    case 2:                     // invalid number of arguments
      INTERFACE.print("<X>");
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_EEPROM
void Sensor::load(){
  struct SensorData data;
  Sensor *tt;

  for(int i=0;i<EEStore::eeStore->data.nSensors;i++){
	  EEPROM.get(EEStore::pointer(),data);
    tt=create(data.snum,data.pin,data.pullUp);
    EEStore::advance(sizeof(tt->data));
  }  
}

///////////////////////////////////////////////////////////////////////////////

void Sensor::store(){
  Sensor *tt;
  
  tt=firstSensor;
  EEStore::eeStore->data.nSensors=0;
  
  while(tt!=NULL){
    EEPROM.put(EEStore::pointer(),tt->data);
    EEStore::advance(sizeof(tt->data));
    tt=tt->nextSensor;
    EEStore::eeStore->data.nSensors++;
  }  
}
#endif

///////////////////////////////////////////////////////////////////////////////

Sensor *Sensor::firstSensor=NULL;

#endif
/**********************************************************************

Accessories.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Accessories.h"

#ifdef USE_ACCESSORIES
#include "TextCommand.h"
#include "DCCpp_Uno.h"
#include "EEStore.h"
#include <EEPROM.h>
#include "Comm.h"

///////////////////////////////////////////////////////////////////////////////

void Turnout::activate(int s){
  char c[20];
  data.tStatus=(s>0);                                    // if s>0 set turnout=ON, else if zero or negative set turnout=OFF
  sprintf(c,"a %d %d %d",data.address,data.subAddress,data.tStatus);
  TextCommand::parse(c);
#ifdef USE_EEPROM
  if(num>0)
    EEPROM.put(num,data.tStatus);
#endif
  INTERFACE.print("<H");
  INTERFACE.print(data.id);
  if(data.tStatus==0)
    INTERFACE.print(" 0>");
  else
    INTERFACE.print(" 1>"); 
}

///////////////////////////////////////////////////////////////////////////////

Turnout* Turnout::get(int n){
  Turnout *tt;
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;tt=tt->nextTurnout);
  return(tt); 
}
///////////////////////////////////////////////////////////////////////////////

void Turnout::remove(int n){
  Turnout *tt,*pp;
  
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextTurnout);

  if(tt==NULL){
    INTERFACE.print("<X>");
    return;
  }
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);

  INTERFACE.print("<O>");
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::show(int n){
  Turnout *tt;

  if(firstTurnout==NULL){
    INTERFACE.print("<X>");
    return;
  }
    
  for(tt=firstTurnout;tt!=NULL;tt=tt->nextTurnout){
    INTERFACE.print("<H");
    INTERFACE.print(tt->data.id);
    if(n==1){
      INTERFACE.print(" ");
      INTERFACE.print(tt->data.address);
      INTERFACE.print(" ");
      INTERFACE.print(tt->data.subAddress);
    }
    if(tt->data.tStatus==0)
       INTERFACE.print(" 0>");
     else
       INTERFACE.print(" 1>"); 
  }
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::parse(char *c){
  int n,s,m;
  Turnout *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
      t=get(n);
      if(t!=NULL)
        t->activate(s);
      else
        INTERFACE.print("<X>");
      break;

    case 3:                     // argument is string with id number of turnout followed by an address and subAddress
      create(n,s,m,1);
    break;

    case 1:                     // argument is a string with id number only
      remove(n);
    break;
    
    case -1:                    // no arguments
      show(1);                  // verbose show
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_EEPROM
void Turnout::load(){
  struct TurnoutData data;
  Turnout *tt;

  for(int i=0;i<EEStore::eeStore->data.nTurnouts;i++){
    EEPROM.get(EEStore::pointer(),data);  
    tt=create(data.id,data.address,data.subAddress);
    tt->data.tStatus=data.tStatus;
    tt->num=EEStore::pointer();
    EEStore::advance(sizeof(tt->data));
  }  
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::store(){
  Turnout *tt;
  
  tt=firstTurnout;
  EEStore::eeStore->data.nTurnouts=0;
  
  while(tt!=NULL){
    tt->num=EEStore::pointer();
    EEPROM.put(EEStore::pointer(),tt->data);
    EEStore::advance(sizeof(tt->data));
    tt=tt->nextTurnout;
    EEStore::eeStore->data.nTurnouts++;
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::create(int id, int add, int subAdd, int v){
  Turnout *tt;
  
  if(firstTurnout==NULL){
    firstTurnout=(Turnout *)calloc(1,sizeof(Turnout));
    tt=firstTurnout;
  } else if((tt=get(id))==NULL){
    tt=firstTurnout;
    while(tt->nextTurnout!=NULL)
      tt=tt->nextTurnout;
    tt->nextTurnout=(Turnout *)calloc(1,sizeof(Turnout));
    tt=tt->nextTurnout;
  }

  if(tt==NULL){       // problem allocating memory
    if(v==1)
      INTERFACE.print("<X>");
    return(tt);
  }
  
  tt->data.id=id;
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  if(v==1)
    INTERFACE.print("<O>");
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::firstTurnout=NULL;
#endif



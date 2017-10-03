/**********************************************************************

Turnout.cpp, renamed from Accessories.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "DCCpp.h"

#ifdef USE_TURNOUT

#include "Turnout.h"
#include "DCCpp_Uno.h"
#include "Comm.h"
#ifdef USE_TEXTCOMMAND
#include "TextCommand.h"
#ifdef USE_EEPROM
#include "EEStore.h"
#include "EEPROM.h"
#endif
#endif

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
#ifdef DCCPP_DEBUG_MODE
  INTERFACE.print("<H");
  INTERFACE.print(data.id);
  if(data.tStatus==0)
    INTERFACE.print(" 0>");
  else
    INTERFACE.print(" 1>"); 
#endif
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
#ifdef DCCPP_DEBUG_MODE
	  INTERFACE.print("<X>");
#endif
    return;
  }
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);

#ifdef DCCPP_DEBUG_MODE
  INTERFACE.print("<O>");
#endif
}

#ifdef DCCPP_PRINT_DCCPP

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
       INTERFACE.println(" 0>");
     else
       INTERFACE.println(" 1>"); 
  }
}
#endif

#ifdef USE_TEXTCOMMAND

///////////////////////////////////////////////////////////////////////////////

void Turnout::parse(char *c){
  int n,s,m;
  Turnout *t;
  
  switch(sscanf(c,"%d %d %d",&n,&s,&m)){
    
    case 2:                     // argument is string with id number of turnout followed by zero (not thrown) or one (thrown)
      t=get(n);
      if(t!=NULL)
        t->activate(s);
#ifdef DCCPP_DEBUG_MODE
	  else
        INTERFACE.print("<X>");
#endif
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
#ifdef DCCPP_DEBUG_MODE
    if(v==1)
      INTERFACE.print("<X>");
#endif
    return(tt);
  }
  
  tt->data.id=id;
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
#ifdef DCCPP_DEBUG_MODE
  if(v==1)
    INTERFACE.print("<O>");
#endif
  return(tt);
  
}

///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::firstTurnout=NULL;
#endif



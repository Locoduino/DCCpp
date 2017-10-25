/**********************************************************************

EEStore.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef EEStore_h
#define EEStore_h

#include "DCCpp.h"

#ifdef USE_EEPROM

#define  EESTORE_ID "DCC++"

struct EEStoreData{
  char id[sizeof(EESTORE_ID)];
#ifdef USE_TURNOUT
  int nTurnouts;
#endif
#ifdef USE_SENSOR
  int nSensors;
#endif
#ifdef USE_OUTPUT
  int nOutputs;
#endif
};

struct EEStore{
  static EEStore *eeStore;
  EEStoreData data;
  static int eeAddress;
  static void init();
  static void reset();
  static int pointer();
  static void advance(int);
  static bool needsRefreshing();
  static void store();
  static void clear();
};
  
#endif
#endif


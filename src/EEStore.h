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

/** This structure stores data of EEStore class.*/
struct EEStoreData{
  char id[sizeof(EESTORE_ID)+1]; /**< String used to check if the data already in the EEPROM was saved by DCC++.*/
#ifdef USE_TURNOUT
  int nTurnouts;	/**< Turnouts number.*/
#endif
#ifdef USE_SENSOR
  int nSensors;	/**< Sensors number.*/
#endif
#ifdef USE_OUTPUT
  int nOutputs;	/**< Outputs number.*/
#endif
};

/** This structure/class stores accessories data in EEPROM.

The data is saved from byte 0 of EEPROM. The content is :

\verbatim
0--------------------------------------+----------+---------+---------+
| EEStoreData(see EEstoreData content) | Turnouts | Sensors | Outputs |
+--------------------------------------+----------+---------+---------+
\endverbatim
*/
struct EEStore{
  //static EEStore *eeStore;	/**< Static pointer to an instance of EEStore class.*/
  static EEStoreData data;	/**< Data associated with this class.*/
  static int eeAddress;	/**< Current EEPROM address. Used during storage.*/

  static void init();	/**< Clear all the EEPROM memory.*/
  static void reset();	/**< Restart the eeAddress pointer at the end of the EEStoreData.*/
  /** Gets the current EEPROM address.
  @return current position in EEPROM memory.
  */
  static int pointer();
  /** Move the EEPROM address of the given value.
  @param inIncrement	number of bytes to shift.
  */
  static void advance(int inIncrement);
  /** Checks if numbers of accessories have changed which needs to store new states.
  @return True if the total number of accessories have changed.
  */
  static bool needsRefreshing();
  static void store();	/**< Stores the current state of all accessories.*/
  static void clear();	/**< Clears the content of the EEStoreData structure. This function does not change the EEPROM.*/
};
  
#endif
#endif

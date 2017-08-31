/**********************************************************************

SerialCommand.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef SerialCommand_h
#define SerialCommand_h

#include "DCCpp.h"

#ifdef USE_SERIALCOMMAND

#include "PacketRegister.h"
#include "CurrentMonitor.h"

#define  MAX_COMMAND_LENGTH         30

/** DCC++ BASE STATION COMMUNICATES VIA THE SERIAL PORT USING SINGLE-CHARACTER TEXT COMMANDS
WITH OPTIONAL PARAMTERS, AND BRACKETED BY < AND > SYMBOLS.  SPACES BETWEEN PARAMETERS
ARE REQUIRED.  SPACES ANYWHERE ELSE ARE IGNORED.  A SPACE BETWEEN THE SINGLE-CHARACTER
COMMAND AND THE FIRST PARAMETER IS ALSO NOT REQUIRED.*/
struct SerialCommand{
  static char commandString[MAX_COMMAND_LENGTH+1];
  static volatile RegisterList *mRegs, *pRegs;
  static CurrentMonitor *mMonitor;
  static void init(volatile RegisterList *, volatile RegisterList *, CurrentMonitor *);
  static void parse(char *);
  static void process();
}; // SerialCommand
  
#endif
#endif





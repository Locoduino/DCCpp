/**********************************************************************

Outputs.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef Outputs_h
#define Outputs_h

#include "DCCpp.h"

#ifdef USE_OUTPUT
#include "Arduino.h"

struct OutputData {
  byte oStatus;
  int id;
  byte pin; 
  byte iFlag; 
};

/**DCC++ BASE STATION supports optional OUTPUT control of any unused Arduino Pins for custom purposes.
Pins can be activited or de-activated.  The default is to set ACTIVE pins HIGH and INACTIVE pins LOW.
However, this default behavior can be inverted for any pin in which case ACTIVE=LOW and INACTIVE=HIGH.

Definitions and state (ACTIVE/INACTIVE) for pins are retained in EEPROM and restored on power-up.
The default is to set each defined pin to active or inactive according to its restored state.
However, the default behavior can be modified so that any pin can be forced to be either active or inactive
upon power-up regardless of its previous state before power-down.

To have this sketch utilize one or more Arduino pins as custom outputs, first define/edit/delete
output definitions using the following variation of the "Z" command:

\verbatim
<Z ID PIN IFLAG>
\endverbatim
creates a new output ID, with specified PIN and IFLAG values.
if output ID already exists, it is updated with specificed PIN and IFLAG.
note: output state will be immediately set to ACTIVE/INACTIVE and pin will be set to HIGH/LOW
according to IFLAG value specifcied (see below).
returns: <O> if successful and <X> if unsuccessful (e.g. out of memory)

\verbatim
<Z ID>
\endverbatim
deletes definition of output ID
returns: <O> if successful and <X> if unsuccessful (e.g. ID does not exist)

\verbatim
<Z>
\endverbatim
lists all defined output pins
returns: <Y ID PIN IFLAG STATE> for each defined output pin or <X> if no output pins defined

where

\verbatim
ID: the numeric ID (0-32767) of the output
PIN: the arduino pin number to use for the output
STATE: the state of the output (0=INACTIVE / 1=ACTIVE)
IFLAG: defines the operational behavior of the output based on bits 0, 1, and 2 as follows:
\endverbatim

-* IFLAG, bit 0:   0 = forward operation (ACTIVE=HIGH / INACTIVE=LOW)
1 = inverted operation (ACTIVE=LOW / INACTIVE=HIGH)

-* IFLAG, bit 1:   0 = state of pin restored on power-up to either ACTIVE or INACTIVE depending
on state before power-down; state of pin set to INACTIVE when first created
1 = state of pin set on power-up, or when first created, to either ACTIVE of INACTIVE
depending on IFLAG, bit 2

-* IFLAG, bit 2:   0 = state of pin set to INACTIVE uponm power-up or when first created
1 = state of pin set to ACTIVE uponm power-up or when first created

Once all outputs have been properly defined, use the <E> command to store their definitions to EEPROM.
If you later make edits/additions/deletions to the output definitions, you must invoke the <E> command if you want those
new definitions updated in the EEPROM.  You can also clear everything stored in the EEPROM by invoking the <e> command.

To change the state of outputs that have been defined use:

\verbatim
<Z ID STATE>
\endverbatim
sets output ID to either ACTIVE or INACTIVE state
returns: <Y ID STATE>, or <X> if turnout ID does not exist

where

\verbatim
ID: the numeric ID (0-32767) of the turnout to control
STATE: the state of the output (0=INACTIVE / 1=ACTIVE)
\endverbatim

When controlled as such, the Arduino updates and stores the direction of each output in EEPROM so
that it is retained even without power.  A list of the current states of each output in the form <Y ID STATE> is generated
by this sketch whenever the <s> status command is invoked.  This provides an efficient way of initializing
the state of any outputs being monitored or controlled by a separate interface or GUI program.
*/
struct Output{
  static Output *firstOutput;
  int num;
  struct OutputData data;
  Output *nextOutput;
  void activate(int s);
  static void parse(char *c);
  static Output* get(int);
  static void remove(int);
#ifdef USE_EEPROM
  static void load();
  static void store();
#endif
  static Output *create(int, int, int, int=0);
  static void show(int=0);
}; // Output
  
#endif
#endif



/**********************************************************************

Outputs.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef Outputs_h
#define Outputs_h

#include "Arduino.h"

#ifdef USE_OUTPUT

/** Data associated to each Output.*/
struct OutputData {
  byte oStatus;	/**< Current activation status.*/
  int id;		/**< The numeric ID (0-32767) of the output.*/
  byte pin;		/**< The Arduino pin number to use for the output.*/
  byte iFlag;	/**< Defines the operational behavior of the output based on bits 0, 1, and 2 (See 'lists all outputs' command for more information).*/	
};

/**	\addtogroup commandsGroup
	CREATES A NEW OUTPU%T
	---------------------

	<b>
	\verbatim
	<Z ID PIN IFLAG>
	\endverbatim
	</b>

	creates a new output ID, with specified PIN and IFLAG values.
	if output ID already exists, it is updated with specified PIN and IFLAG.
	note: output state will be immediately set to ACTIVE/INACTIVE and pin will be set to HIGH/LOW
	according to IFLAG value specified (see output class comment).

	- <b>ID</b>: the numeric ID (0-32767) of the output
	- <b>PIN</b>: the arduino pin number to use for the output
	- <b>STATE</b>: the state of the output (0=INACTIVE / 1=ACTIVE)
	- <b>IFLAG</b>: defines the operational behavior of the output based on bits 0, 1, and 2 (see 'lists all outputs' command for more information).

	returns: <b>\<O\></b> if successful and <b>\<X\></b> if unsuccessful (e.g. out of memory)
*/

/**	\addtogroup commandsGroup
	DELETES AN EXISTING OUTPU%T
	---------------------------

	<b>
	\verbatim
	<Z ID>
	\endverbatim
	</b>

	deletes definition of output ID

	- <b>ID</b>: the numeric ID (0-32767) of the output
	
	returns: <b>\<O\></b> if successful and <b>\<X\></b> if unsuccessful (e.g. ID does not exist)
*/

/**	\addtogroup commandsGroup
	LISTS ALL OUTPUTS
	-----------------

	<b>
	\verbatim
	<Z>
	\endverbatim
	</b>

	lists all defined output pins

	returns: <b>\<Y ID PIN IFLAG STATE\></b> for each defined output pin or <b>\<X\></b> if no output pins defined where

	- <b>ID</b>: the numeric ID (0-32767) of the output
	- <b>PIN</b>: the arduino pin number to use for the output
	- <b>STATE</b>: the state of the output (0=INACTIVE / 1=ACTIVE)
	- <b>IFLAG</b>: defines the operational behavior of the output based on bits 0, 1, and 2 as follows:
		+ IFLAG, bit 0:   0 = forward operation (ACTIVE=HIGH / INACTIVE=LOW)
	1 = inverted operation (ACTIVE=LOW / INACTIVE=HIGH)
		+ IFLAG, bit 1:   0 = state of pin restored on power-up to either ACTIVE or INACTIVE depending
	on state before power-down; state of pin set to INACTIVE when first created
	1 = state of pin set on power-up, or when first created, to either ACTIVE of INACTIVE
	depending on IFLAG, bit 2
		+ IFLAG, bit 2:   0 = state of pin set to INACTIVE upon power-up or when first created
	1 = state of pin set to ACTIVE upon power-up or when first created
*/

/**	\addtogroup commandsGroup
	CHANGES OUTPU%T STATE
	--------------------

	<b>
	\verbatim
	<Z ID STATE>
	\endverbatim
	</b>

	sets output ID to either ACTIVE or INACTIVE state

	- <b>ID</b>: the numeric ID (0-32767) of the output
	- <b>PIN</b>: the arduino pin number to use for the output
	- <b>STATE</b>: the state of the output (0=INACTIVE / 1=ACTIVE)

	returns: <b>\<Y ID STATE\></b>, or <b>\<X\></b> if turnout ID does not exist
*/

/**DCC++ BASE STATION supports optional OUTPUT control of any unused Arduino Pins for custom purposes.
Pins can be activated or dis-activated.  The default is to set ACTIVE pins HIGH and INACTIVE pins LOW.
However, this default behavior can be inverted for any pin in which case ACTIVE=LOW and INACTIVE=HIGH.

Definitions and state (ACTIVE/INACTIVE) for pins are retained in EEPROM and restored on power-up.
The default is to set each defined pin to active or inactive according to its restored state.
However, the default behavior can be modified so that any pin can be forced to be either active or inactive
upon power-up regardless of its previous state before power-down.

To have this sketch utilize one or more Arduino pins as custom outputs, first define/edit/delete
output definitions using the following variation of the "Z" command.

When controlled as such, the Arduino updates and stores the direction of each output in EEPROM so
that it is retained even without power.  A list of the current states of each output in the form <b>\<Y ID STATE\></b> is generated
by this sketch whenever the <b>\<s\></b> status command is invoked.  This provides an efficient way of initializing
the state of any outputs being monitored or controlled by a separate interface or GUI program.
*/
struct Output{
  struct OutputData data;	/**< Data associated to this output.*/

  /** Initialize the output created by regular C++ instantiation.
  @param id	The numeric ID (0-32767) of the output.
  @param pin The arduino pin number to use for the output.
  @param iFlag Defines the operational behavior of the output based on bits 0, 1, and 2 (see in class description for values).
  @remark Should not be used in command line mode.
  */
  void begin(int id, int pin, int iFlag);
  /** Force the output values created by command line.
  @param id	The numeric ID (0-32767) of the output.
  @param pin The arduino pin number to use for the output.
  @param iFlag Defines the operational behavior of the output based on bits 0, 1, and 2 (see in class description for values).
  @remark Should not be used in C++ mode.
  */
  void set(int id, int pin, int iFlag);
  /** Change the activation state of the output. The state of the pin is updated by this function.
  @param s new state : see iFlag description in class description..
  */
  void activate(int s = 1);
  /** Checks if the output is activated or not.
  @return True if the output is activated. Otherwise false (of course...).
  */
  bool isActivated() { return this->data.oStatus > 0; }

#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
  static Output *firstOutput;	/**< Address of the first object of this class. NULL means empty list of outputs. Do not change it !*/
  Output *nextOutput;			/**< Address of the next object of this class. NULL means end of the list of outputs. Do not change it !*/
  /** Get a particular output.
  @param id	The numeric ID (0-32767) of the output.
  @return The found output or NULL if not found.
  @remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
  */
  static Output* get(int id);	
  /** Removes a particular output.
  @param id	The numeric ID (0-32767) of the output.
  @remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
  */
  static void remove(int id);
  /** Get the total number of outputs.
  @return Number of outputs.
  @remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
  */
  static int count();

#ifdef DCCPP_PRINT_DCCPP
  /** Show the content of all the outputs.
  @remark Only available when DCCPP_PRINT_DCCPP is defined.
  */
  static void show();
#endif

#if defined(USE_EEPROM)
  int num;	/**< EEPROM starting address for this Output storage.*/

  /** Load the content of all outputs from EEPROM.
  @remark Only available when USE_EEPROM is defined.
  */
  static void load();
  /** Store the content of all outputs in EEPROM.
  @remark Only available when USE_EEPROM is defined.
  */
  static void store();
#endif

#endif

#if defined(USE_TEXTCOMMAND)
  /** Do command according to the given command string.
  @param c string to interpret... See syntax in class description.
  @remark Only available when USE_TEXTCOMMAND is defined.
  */
  static bool parse(char *c);
  /** Creates a new output in the list..
  @param id	The numeric ID (0-32767) of the output.
  @param pin The arduino pin number to use for the output.
  @param iFlag Defines the operational behavior of the output based on bits 0, 1, and 2 (see in class description for values).
  @remark Should not be used in C++ mode.
  @remark Only available when USE_TEXTCOMMAND is defined.
  */
  static Output *create(int id, int pin, int iFlag);
#endif

}; // Output
  
#endif
#endif

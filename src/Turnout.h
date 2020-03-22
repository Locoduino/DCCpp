/**********************************************************************

Turnout.h renamed from Accessories.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef Turnout_h
#define Turnout_h

#include "Arduino.h"

#include "DCCpp.h"

#ifdef USE_TURNOUT
/** Data associated to each Turnout.*/
struct TurnoutData {
  byte tStatus;			/**< Current status of the turnout : 0 for Off, 1 for On.*/
  byte subAddress;		/**< The sub-address of the decoder controlling this turnout (0-3).*/
  int id;				/**< The numeric ID (0-32767) of the turnout to control.*/
  int address;  		/**< The primary address of the decoder controlling this turnout (0-511).*/
};

/**	\addtogroup commandsGroup
	CREATES A NEW TURNOU%T
	----------------------

	<b>
	\verbatim
	<T ID ADDRESS SUBADDRESS>
	\endverbatim
	</b>

	creates a new turnout ID, with specified ADDRESS and SUBADDRESS
	if turnout ID already exists, it is updated with specified ADDRESS and SUBADDRESS

	- <b>ID</b>: the numeric ID (0-32767) of the turnout to control
	- <b> ADDRESS</b>:  the primary address of the decoder controlling this turnout (0-511)
	- <b> SUBADDRESS</b>: the sub-address of the decoder controlling this turnout (0-3)

	returns: <b>\<O\></b> if successful and <b>\<X\></b> if unsuccessful (e.g. out of memory)
*/

/**	\addtogroup commandsGroup
	DELETES AN EXISTING TURNOU%T
	----------------------------

	<b>
	\verbatim
	<T ID>
	\endverbatim
	</b>

	deletes definition of turnout ID

	- <b>ID</b>: the numeric ID (0-32767) of the turnout to control

	returns: <b>\<O\></b> if successful and <b>\<X\></b> if unsuccessful (e.g. ID does not exist)
*/

/**	\addtogroup commandsGroup
	LISTS ALL TURNOUTS
	------------------

	<b>
	\verbatim
	<T>
	\endverbatim
	</b>

	returns: <b>\<H ID ADDRESS SUBADDRESS THROW\></b> for each defined turnout or <b>\<X\></b> if no turnouts defined where

	- <b>ID</b>: the numeric ID (0-32767) of the turnout to control
	- <b>ADDRESS</b>:  the primary address of the decoder controlling this turnout (0-511)
	- <b>SUBADDRESS</b>: the sub-address of the decoder controlling this turnout (0-3)
*/

/**	\addtogroup commandsGroup
	THROWS AN EXISTING TURNOU%T
	----------------------------

	<b>
	\verbatim
	<T ID THROW>
	\endverbatim
	</b>

	sets turnout ID to either the "thrown" or "not thrown" position

	- <b>ID</b>: the numeric ID (0-32767) of the turnout to control
	- <b>THROW</b>:  0 (not thrown) or 1 (thrown)

	returns: <b>\<H ID THROW\></b>, or <b>\<X\></b> if turnout ID does not exist
*/

/** DCC++ BASE STATION can keep track of the direction of any turnout that is controlled
by a DCC stationary accessory decoder.

If the TextCommand is used, all turnouts, as well as any other DCC accessories
connected in this fashion, can always be operated using the DCC BASE STATION Turnout command.

When controlled as such, the Arduino updates and stores the direction of each Turnout in EEPROM so
that it is retained even without power.  A list of the current directions of each Turnout in the form <b>\<H ID THROW\></b> is generated
by this sketch whenever the <b>\<s\></b> status command is invoked.  This provides an efficient way of initializing
the directions of any Turnouts being monitored or controlled by a separate interface or GUI program.

Once all turnouts have been properly defined, use the <b>\<E\></b> command to store their definitions to EEPROM.
If you later make edits/additions/deletions to the turnout definitions, you must invoke the <b>\<E\></b> command if you want those
new definitions updated in the EEPROM.  You can also clear everything stored in the EEPROM by invoking the <b>\<e\></b> command.

Without	TextCommand, this is a classic C++ usage. An instance of the structure is created by the user,
and functions like begin() and activate() can be applied on this instance. 
load() and store() of the class can still be used (if USE_EEPROM is activated).
*/
struct Turnout{
	struct TurnoutData data;	/**< Data associated to this output.*/

	/** Initialize the turnout created by regular C++ instantiation.
	@param id The numeric ID (0-32767) of the sensor.
	@param add	The primary address of the decoder controlling this turnout (0-511).
	@param subAdd	The sub-address of the decoder controlling this turnout (0-3).
	@remark Should not be used in command line mode.
	*/
	void begin(int id, int add, int subAdd);
	/** Force the turnout values created by command line.
	@param id The numeric ID (0-32767) of the sensor.
	@param add	The primary address of the decoder controlling this turnout (0-511).
	@param subAdd	The sub-address of the decoder controlling this turnout (0-3).
	@remark Should not be used in C++ mode.
	*/
	void set(int id, int add, int subAdd);
	/** Change the activation state of the turnout.
	@param s new state : 0 for off, 1 for on. Default is 1.
	*/
	void activate(int s = 1);
	/** Inactivate the turnout.
	*/
	inline void inactivate() { activate(0); }
	/** Checks if the output is activated or not.
	@return True if the output is activated. Otherwise false (of course...).
	*/
	inline bool isActivated() { return this->data.tStatus > 0; }

#if defined(USE_EEPROM)	|| defined(USE_TEXTCOMMAND)
	static Turnout *firstTurnout;	/**< Address of the first object of this class. NULL means empty list of turnouts. Do not change it !*/
	Turnout *nextTurnout;			/**< Address of the next object of this class. NULL means end of the list of turnouts. Do not change it !*/
	/** Get a particular turnout.
	@param id	The numeric ID (0-32767) of the turnout.
	@return The found turnout or NULL if not found.
	@remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
	*/
	static Turnout* get(int id);
	/** Removes a particular turnout.
	@param id	The numeric ID (0-32767) of the turnout.
	@remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
	*/
	static void remove(int id);
	/** Get the total number of turnouts.
	@return Number of turnouts.
	@remark Only available when USE_EEPROM or USE_TEXTCOMMAND is defined.
	*/
	static int count();

#ifdef DCCPP_PRINT_DCCPP
	/** Show the content of all the turnouts.
	@remark Only available when DCCPP_PRINT_DCCPP is defined.
	*/
	static void show();
#endif

#if defined(USE_EEPROM)
	int eepromPos;	/**< EEPROM starting address for this Turnout storage.*/

	/** Load the content of all turnouts from EEPROM.
	@remark Only available when USE_EEPROM is defined.
	*/
	static void load();
	/** Store the content of all turnouts in EEPROM.
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
	/** Creates a new turnout in the list..
	@param id The numeric ID (0-32767) of the sensor.
	@param add	The primary address of the decoder controlling this turnout (0-511).
	@param subAdd	The sub-address of the decoder controlling this turnout (0-3).
	@remark Should not be used in C++ mode.
	@remark Only available when USE_TEXTCOMMAND is defined.
	*/
	static Turnout *create(int id, int add, int subAdd);
#endif

}; // Turnout
  
#endif
#endif

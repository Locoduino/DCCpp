//-------------------------------------------------------------------
#ifndef __Registers_hpp__
#define __Registers_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#define MAXIMUM_REGISTER_SIZE		128

/**	This is the static class for assigned DCC++ register list.
Register numbers are valid between 1 and 127.
0 is not an allowed register number, and is used an error flag.
*/
class Registers
{
private:
	/**16 bytes : 128 bits : from register 0 to 127, maximum for DCCpp.
	A bit at true is the corresponding register is in use.*/
	static byte registers[MAXIMUM_REGISTER_SIZE / 8];

public:
	/** Clears the complezte list of registers.
	*/
	static void clear();
	/** Try to get one free register.
	@return 0 if no register found, otherwise the allocated register number.
	*/
	static uint8_t allocateRegister();
	/** Free the given register.
	@param inRegister	Register number between 1 and 127.
	*/
	static void freeRegister(uint8_t inRegister);
	/** Check if the given register is allocated or not.
	@param inRegister	Register number between 1 and 127.
	*/
	static bool isAllocated(byte inRegister);

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of assigned locomotives.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void printRegisters();
#endif
};
#endif
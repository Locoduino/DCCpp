//-------------------------------------------------------------------
#ifndef __Locomotives_hpp__
#define __Locomotives_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#include "FunctionsState.hpp"
#include "Locomotive.hpp"
#include "Registers.hpp"

/**	This is the static class for assigned locomotive list.
*/
class Locomotives
{
private:
	static Locomotive* pFirstLocomotive;	/**< Address of the first object of this class. NULL means empty list of Locomotives. Do not change it !*/
	static bool useFunctionRegister;

public:
	/** Set the flag to use two registers by locomotive to keep alive functions after a long current cut.
	@param inUse	True if each locomotive must use one register for speed and another one for functions.
	*/
	static void setUseFunctionRegister(bool inUse) { useFunctionRegister = inUse; }
	/** Get a particular Locomotive.
	@param inAddress	The DCC id (1-65535) of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* get(uint16_t inAddress);
	/** Removes a particular locomotive.
	@param inAddress	The DCC id (1-65535) of the locomotive.
	*/
	static void remove(uint16_t inAddress);
	/** Get a particular Locomotive.
	@param inName	The name of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* get(const char* inName);
	/** Removes a particular locomotive.
	@param inName	The name of the locomotive.
	*/
	static void remove(const char* inName);
	/** Get the maximum number of locomotives.
	@return Number of locomotives.
	*/
	static int countMax() {	return (MAX_MAIN_REGISTERS - 1) % 2; }
	/** Get the total number of assigned locomotives.
	@return Number of locomotives.
	*/
	static int count();

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of assigned locomotives.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void printLocomotives();
#endif
};
#endif
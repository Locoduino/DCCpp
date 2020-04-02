//-------------------------------------------------------------------
#ifndef __FunctionsState_hpp__
#define __FunctionsState_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

/**
This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class FunctionsState
{
private:
	/**Four bytes : 32 bits : from function 0 to 28, maximum for DCC.
	A bit at true is an activated function.*/
	byte activeFlags[4];

	/**This is the last states sent to the decoder by DCC.
	Used to be sure to send only changed states.
	*/
	byte activeFlagsSent[4];

	inline byte byteNumber(byte inFunctionNumber) { return inFunctionNumber / 8; }
	inline byte bitNumber(byte inFunctionNumber) { return inFunctionNumber % 8; }

public:
	/** Initialize the instance.
	*/
	FunctionsState();

	/** Reset all functions to inactive.
	*/
	void clear();
	/** Activate one function. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to activate.
	*/
	void activate(byte inFunctionNumber);
	/** Inactivate one function. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to inactivate.
	*/
	void inactivate(byte inFunctionNumber);
	/** Check if the given function is activated. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to activate.
	@return True if the given function is activated.
	*/
	bool isActivated(byte inFunctionNumber);
	/**Copy the current active flags into 'sent' active flags.*/
	void statesSent();
	/** Check if the given function had its activation flag changed when sent to the decoder last time.
	The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to check.
	@return True if the given function activation flag is different between activFlags and sentActiveFlags.
	*/
	bool isActivationChanged(byte inFunctionNumber);

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of activated functions.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printActivated();
#endif
};

#endif
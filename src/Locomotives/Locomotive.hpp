//-------------------------------------------------------------------
#ifndef __Locomotive_hpp__
#define __Locomotive_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#define LOCOMOTIVE_NAME_SIZE	32

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class Locomotive
{
private:
	char name[LOCOMOTIVE_NAME_SIZE];
	uint16_t address;	// 10239 is the DCC norm maximum.	9983 for Digitrax, 9999 for NCE and Lenz.
	int8_t speedRegisterNumber;
	int8_t functionRegisterNumber;
	uint8_t speedMax;
	uint8_t currentSpeed;
	bool direction;
	
	FunctionsState functions;

public:
	Locomotive* pNextLocomotive;					/**< Address of the next object of this class. NULL means end of the list of Locomotives. Do not change it !*/

	/** Creates a new instance for only one register.
	*/
	Locomotive(const char* inName, uint8_t speedRegister, uint16_t inAddress = 3, uint8_t inSpeedMax = 128);

	/** Creates a new instance for two registers.
	*/
	Locomotive(const char* inName, uint8_t speedRegister, uint8_t functionRegister, uint16_t inAddress = 3, uint8_t inSpeedMax = 128);

	/** Sets the locomotive DCC address
	@param inAddress	Locomotive new DCC address.
	*/
	void setAddress(uint16_t inAddress) { this->address = inAddress; }
	/** Gets the locomotive DCC address.
	@return	Locomotive DCC address.
	*/
	uint16_t getAddress() { return this->address; }
	/** Sets the locomotive name
	@param inNames	Locomotive new name.
	*/
	void setName(const char * inName) { strncpy(this->name, inName, LOCOMOTIVE_NAME_SIZE); }
	/** Gets the locomotive name.
	@return	Locomotive name.
	*/
	const char *getName() const { return this->name; }

	/** Sets the locomotive max speed : 14, 28 or 128.
	@param inSpeedMax	Locomotive max speed steps, default is 128.
	@remark	any other value than 14, 28 or 128 will result in a 128 speed steps value.
	*/
	void setSpeedMax(uint8_t inSpeedMax) { if (inSpeedMax == 14 || inSpeedMax == 28 || inSpeedMax == 128) this->speedMax = inSpeedMax; else this->speedMax = 128; }
	/** Gets the locomotive max speed : 14, 28 or 128.
	@return	Locomotive max speed steps : 14, 28 or 128.
	*/
	uint8_t getSpeedMax() { return this->speedMax; }

	/** Sets the locomotive current speed.
	@param inSpeed	Locomotive new speed.
	@remark	For 128 steps max, 0 is stop, 1 is emergency stop...
	*/
	void setSpeed(uint8_t speed) {
		if (speed < 2) { speed = 2;	}
		else if (speed > 127) {	speed = 127; }
		this->currentSpeed = speed;
	}

	/** Gets the locomotive current speed.
	@return	Locomotive current speed.
	*/
	uint8_t getSpeed() {		return this->currentSpeed; }

	void stop() { this->setSpeed(0); }
	void emergencyStop() { this->setSpeed(1); }

	/** Sets the locomotive direction.
	@param inForward	True if the locomotive must go forward.
	*/
	void setDirection(bool inForward) {		this->direction = inForward;	}
	/** Check if the locomotive direction is forward.
	@return True if the locomotive direction is forward.
	*/
	bool isDirectionForward() {		return this->direction;	}

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the locomotive.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printLocomotive();
#endif
};

#endif
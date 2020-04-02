/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

Locomotive::Locomotive(const char* inName, uint8_t inSpeedRegister, uint16_t inAddress, uint8_t inSpeedMax)
{
	// 'static' data, not updated during the run.
	strncpy(this->name, inName, LOCOMOTIVE_NAME_SIZE);
	this->address = inAddress;
	this->speedMax = inSpeedMax;
	this->speedRegisterNumber = inSpeedRegister;
	this->functionRegisterNumber = 0;

	// Variable data
	this->currentSpeed = 0; // regular stop
	this->direction = true;	// goes forward
	this->pNextLocomotive = NULL;
}

Locomotive::Locomotive(const char* inName, uint8_t inSpeedRegister, uint8_t inFunctionRegister, uint16_t inAddress, uint8_t inSpeedMax)
{
	// 'static' data, not updated during the run.
	strncpy(this->name, inName, LOCOMOTIVE_NAME_SIZE);
	this->address = inAddress;
	this->speedMax = inSpeedMax;
	this->speedRegisterNumber = inSpeedRegister;
	this->functionRegisterNumber = 0;

	// Variable data
	this->currentSpeed = 0; // regular stop
	this->direction = true;	// goes forward
	this->pNextLocomotive = NULL;
}

#ifdef DCCPP_DEBUG_MODE
void Locomotive::printLocomotive()
{
	Serial.print(this->name);
	Serial.print(" reg:");
	Serial.print(this->speedRegisterNumber);
	Serial.print(" id:");
	Serial.print(this->address);
	Serial.print(" max:");
	Serial.print(this->speedMax);

	Serial.print("      +/-speed:");
	Serial.print(this->currentSpeed * (this->direction == true?1:-1));

	Serial.println("");
}
#endif

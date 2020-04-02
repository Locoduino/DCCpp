/*************************************************************
project: <DCCpp library extension>
author: <Thierry PARIS>
description: <Registers class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

byte Registers::registers[MAXIMUM_REGISTER_SIZE / 8];

void Registers::clear()
{
	for (int i = 0; i < MAXIMUM_REGISTER_SIZE / 8; i++)
		registers[i] = 0;
}

uint8_t Registers::allocateRegister()
{
	for (int i = 1; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		if (bitRead(registers[i / 8], i % 8) == 0)
		{
			bitSet(registers[i / 8], i % 8);
			return i;
		}
	}

	return 0;	// No register free !
}

void Registers::freeRegister(uint8_t inRegister)
{
	bitClear(registers[inRegister / 8], inRegister % 8);
}

bool Registers::isAllocated(byte inRegister)
{
	return bitRead(registers[inRegister / 8], inRegister % 8);
}

#ifdef DCCPP_DEBUG_MODE
void Registers::printRegisters()
{
	for (int i = 0; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		if (isAllocated(i))
		{
			Serial.print(i);
			Serial.print(" ");
		}
	}

	Serial.println("");
}
#endif

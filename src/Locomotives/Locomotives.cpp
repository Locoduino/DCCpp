/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

Locomotive* Locomotives::pFirstLocomotive = NULL;
bool Locomotives::useFunctionRegister = false;

Locomotive* Locomotives::get(uint16_t inAddress)
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		if (pCurr->getAddress() == inAddress)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextLocomotive;
	}

	return NULL;
}

Locomotive* Locomotives::get(const char* inName)
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		if (strcmp(pCurr->getName(), inName) == 0Boxc)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextLocomotive;
	}

	return NULL;
}

void Locomotives::remove(uint16_t inAddress)
{
}

void Locomotives::remove(const char* inName)
{
}

int Locomotives::count()
{
	uint8_t count = 0;

	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		count++;
		pCurr = pCurr->pNextLocomotive;
	}

	return count;
}

#ifdef DCCPP_DEBUG_MODE
/** Print the list of assigned locomotives.
@remark Only available if DCCPP_DEBUG_MODE is defined.
*/
static void printLocomotives();
#endif

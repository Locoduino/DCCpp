/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc Serial Controller sample>
*************************************************************/

#include "DCCpp.h"

#ifndef USE_TEXTCOMMAND
#error To be able to compile this sample,the line #define USE_TEXTCOMMAND must be uncommented in DCCpp.h
#endif

void setup()
{
	Serial.begin(115200);

	DCCpp::begin();
	// Configuration for my LMD18200. See the page 'Configuration lines' in the documentation for other samples.
	DCCpp::beginMain(10, 11, A0);
}

void loop()
{
	DCCpp::loop();
}


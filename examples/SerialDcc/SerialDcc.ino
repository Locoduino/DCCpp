/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc Serial Controller sample>
*************************************************************/

#include "DCCpp.h"

void setup()
{
	Serial.begin(115200);

	DCCpp.begin();
	DCCpp.beginMain(255, DCC_SIGNAL_PIN_MAIN, 11, A6);    // Dc: Dir, Pwm, current sensor
}

void loop()
{
	DCCpp.loop();
}


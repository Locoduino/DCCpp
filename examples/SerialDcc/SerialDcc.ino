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

	DCCpp.begin();
	DCCpp.beginMain(255, 12, 50, A5);
	//DCCpp.beginProg(255, MOTOR_SHIELD_DIRECTION_MOTOR_CHANNEL_PIN_B, MOTOR_SHIELD_SIGNAL_ENABLE_PIN_PROG, MOTOR_SHIELD_CURRENT_MONITOR_PIN_PROG);
	//DCCpp.beginMain(255, DCC_SIGNAL_PIN_MAIN, 11, A6);
	Serial.println("Setup finished");
}

void loop()
{
	DCCpp.loop();
}


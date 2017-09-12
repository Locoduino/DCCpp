/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dc/Dcc Nano Controller sample>
*************************************************************/

#include "Commanders.h"

#include "DCCpp.h"

#define EVENT_NONE		0
#define EVENT_MORE		1
#define EVENT_LESS		2
#define EVENT_SELECT	3
#define EVENT_CANCEL	4
#define EVENT_MOVE		5
#define EVENT_START		6
#define EVENT_END		7
#define EVENT_EMERGENCY	8
#define EVENT_FUNCTION0	9
#define EVENT_FUNCTION1	10
#define EVENT_ENCODER	11


#ifdef VISUALSTUDIO
ButtonsCommanderKeyboard push0;
ButtonsCommanderKeyboard push1;
ButtonsCommanderKeyboard push2;
ButtonsCommanderKeyboard push3;
ButtonsCommanderKeyboard pushEmergency;
ButtonsCommanderKeyboard pushFunction1;
ButtonsCommanderKeyboard pushFunction2;
#endif
ButtonsCommanderPush buttonSelect;
ButtonsCommanderEncoder buttonEncoder;
ButtonsCommanderPush buttonCancel;
ButtonsCommanderPush buttonEmergency;
ButtonsCommanderSwitchOnePin buttonF0;
ButtonsCommanderSwitchOnePin buttonF1;


// in this sample, only one loco is driven...
int locoId;	// DCC id for this loco
int locoStepsNumber;	// 14, 28 or 128
int locoSpeed;	// Current speed
bool locoDirectionToLeft;	// current direction.
FunctionsState locoFunctions;	// Current functions

void setup()
{
	Serial.begin(115200);

#ifdef VISUALSTUDIO
	push0.begin(EVENT_SELECT, '*');
	push1.begin(EVENT_MORE, '+');
	push2.begin(EVENT_LESS, '-');
	push3.begin(EVENT_CANCEL, '/');
	pushEmergency.begin(EVENT_EMERGENCY, '0');
	pushFunction1.begin(EVENT_FUNCTION0, '1', COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_ON);
	pushFunction1.AddEvent(EVENT_FUNCTION0, COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_OFF);
	pushFunction2.begin(EVENT_FUNCTION1, '2', COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_ON);
	pushFunction2.AddEvent(EVENT_FUNCTION1, COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_OFF);
#endif
	buttonSelect.begin(EVENT_SELECT, A0);
	buttonEncoder.begin(EVENT_ENCODER, 14, 8, 2);
	buttonCancel.begin(EVENT_CANCEL, A3);
	buttonEmergency.begin(EVENT_EMERGENCY, A4);
	buttonF0.begin(EVENT_FUNCTION0, A1);
	buttonF1.begin(EVENT_FUNCTION1, A2);

#ifdef VISUALSTUDIO
	pinName(A0, "OK");
	pinName(A1, "F0");
	pinName(A2, "F1");
	pinName(A3, "CANC");
	pinName(A4, "STOP");
	pinName(A5, "DCC");
#endif

	DCCpp.begin();
	DCCpp.beginMain(255, DCC_SIGNAL_PIN_MAIN, 11, A6);    // Dc: Dir, Pwm, current sensor

	locoId = 3;
	locoStepsNumber = 128;
	locoSpeed = 0;
	locoDirectionToLeft = false;
	//locoFunctions.Clear();	// Already done by the constructor...
}

void loop()
{
	DCCpp.loop();
	unsigned long event = Commanders::loop();

	switch (event)
	{
	case EVENT_MORE:
		if (locoStepsNumber >= 100)
			locoSpeed += 10;
		else
			locoSpeed++;
		if (locoSpeed > locoStepsNumber)
			locoSpeed = locoStepsNumber;
		DCCpp.SetSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionToLeft);
		break;

	case EVENT_LESS:
		if (locoStepsNumber >= 100)
			locoSpeed -= 10;
		else
			locoSpeed--;
		if (locoSpeed < 0)
			locoSpeed = 0;
		DCCpp.SetSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionToLeft);
		break;

	case EVENT_FUNCTION0:
		if (locoFunctions.IsActivated(0))
			locoFunctions.Inactivate(0);
		else
			locoFunctions.Activate(0);
		DCCpp.SetFunctionsMain(2, locoId, locoFunctions);
		break;

	case EVENT_FUNCTION1:
		if (locoFunctions.IsActivated(1))
			locoFunctions.Inactivate(1);
		else
			locoFunctions.Activate(1);
		DCCpp.SetFunctionsMain(3, locoId, locoFunctions);
		break;
	}
}


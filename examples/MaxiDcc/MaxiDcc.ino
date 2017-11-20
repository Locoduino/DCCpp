/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc++ Controller sample with all options>
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

#define EVENT_TURNOUT1	20
#define EVENT_TURNOUT2	21

ButtonsCommanderPush buttonSelect;
ButtonsCommanderEncoder buttonEncoder;
ButtonsCommanderPush buttonCancel;
ButtonsCommanderPush buttonEmergency;
ButtonsCommanderSwitchOnePin buttonF0;
ButtonsCommanderSwitchOnePin buttonF1;
ButtonsCommanderSwitchOnePin buttonTurnout1;
ButtonsCommanderSwitchOnePin buttonTurnout2;

// in this sample, only one loco is driven...
int locoId;	// DCC id for this loco
int locoStepsNumber;	// 14, 28 or 128
int locoSpeed;	// Current speed
bool locoDirectionToLeft;	// current direction.
FunctionsState locoFunctions;	// Current functions

Turnout turn1, turn2;
Output output1, output2;
Sensor sensor1, sensor2;

void setup()
{
	Serial.begin(115200);

	buttonSelect.begin(EVENT_SELECT, A0);
	buttonEncoder.begin(EVENT_ENCODER, 14, 8, 2);
	buttonCancel.begin(EVENT_CANCEL, A3);
	buttonEmergency.begin(EVENT_EMERGENCY, A4);
	buttonF0.begin(EVENT_FUNCTION0, A1);
	buttonF1.begin(EVENT_FUNCTION1, A2);
	buttonTurnout1.begin(EVENT_TURNOUT1, 30);
	buttonTurnout2.begin(EVENT_TURNOUT2, 31);

	DCCpp.begin();
	DCCpp.beginMain(255, DCC_SIGNAL_PIN_MAIN, 11, A6);    // Dc: Dir, Pwm, current sensor
	DCCpp.beginProg(255, DCC_SIGNAL_PIN_PROG, 3, A5);    // Dc: Dir, Pwm, current sensor

	locoId = 3;
	locoStepsNumber = 128;
	locoSpeed = 0;
	locoDirectionToLeft = false;
	//locoFunctions.Clear();	// Already done by the constructor...

	turn1.begin(1, 100, 1);
	turn2.begin(2, 200, 2);

	output1.begin(1, 5, B110);
	output2.begin(2, 6, B110);

	sensor1.begin(1, 7, 0);
	sensor2.begin(2, 9, 1);
}

void loop()
{
	DCCpp.loop();

	// activate first output from first sensor state.
	int active = sensor1.isActive();

	if (active != output1.data.oStatus)
		output1.activate(active);

	// activate second output from second sensor state.
	active = sensor2.isActive();

	if (active != output2.data.oStatus)
		output2.activate(active);
		
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

	case EVENT_TURNOUT1:
		if (turn1.isActivated())
			turn1.inactivate();
		else
			turn1.activate();
		break;

	case EVENT_TURNOUT2:
		if (turn2.isActivated())
			turn2.inactivate();
		else
			turn2.activate();
		break;
	}
}


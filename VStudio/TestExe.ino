/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dc/Dcc Nano Controller sample>
*************************************************************/

#include "Commanders.h"
#include "EEPROM.h"

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

#define EVENT_TURNOUT1	12
#define EVENT_TURNOUT2	13
#define EVENT_TURNOUT3	14


#ifdef VISUALSTUDIO
ButtonsCommanderKeyboard push0;
ButtonsCommanderKeyboard push1;
ButtonsCommanderKeyboard push2;
ButtonsCommanderKeyboard push3;
ButtonsCommanderKeyboard pushEmergency;
ButtonsCommanderKeyboard pushFunction1;
ButtonsCommanderKeyboard pushFunction2;
ButtonsCommanderKeyboard pushTurnout1;
ButtonsCommanderKeyboard pushTurnout2;
ButtonsCommanderKeyboard pushTurnout3;
#endif
ButtonsCommanderPush buttonSelect;
ButtonsCommanderEncoder buttonEncoder;
ButtonsCommanderPush buttonCancel;
ButtonsCommanderPush buttonEmergency;
ButtonsCommanderSwitchOnePin buttonF0;
ButtonsCommanderSwitchOnePin buttonF1;
ButtonsCommanderSwitchOnePin buttonT1;
ButtonsCommanderSwitchOnePin buttonT2;
ButtonsCommanderSwitchOnePin buttonT3;

Turnout t1, t2, t3;
Output o1, o2, o3;
Sensor s1, s2, s3;

// in this sample, only one loco is driven...
int locoId;	// DCC id for this loco
int locoStepsNumber;	// 14, 28 or 128
int locoSpeed;	// Current speed
bool locoDirectionForward;	// current direction.
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
	pushTurnout1.begin(EVENT_TURNOUT1, 'a', COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_ON);
	pushTurnout1.AddEvent(EVENT_TURNOUT1, COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_OFF);
	pushTurnout2.begin(EVENT_TURNOUT2, 'z', COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_ON);
	pushTurnout2.AddEvent(EVENT_TURNOUT2, COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_OFF);
	pushTurnout3.begin(EVENT_TURNOUT3, 'e', COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_ON);
	pushTurnout3.AddEvent(EVENT_TURNOUT3, COMMANDERS_EVENT_MOVE, COMMANDERS_MOVE_OFF);
#endif
	buttonSelect.begin(EVENT_SELECT, A0);
	buttonEncoder.begin(EVENT_ENCODER, 14, 8, 2);
	buttonCancel.begin(EVENT_CANCEL, A3);
	buttonEmergency.begin(EVENT_EMERGENCY, A4);
	buttonF0.begin(EVENT_FUNCTION0, A1);
	buttonF1.begin(EVENT_FUNCTION1, A2);
	buttonT1.begin(EVENT_TURNOUT1, 20);
	buttonT2.begin(EVENT_TURNOUT2, 21);
	buttonT3.begin(EVENT_TURNOUT3, 22);

	t1.begin(1, 10, 101);
	t2.begin(2, 20, 201);
	t3.begin(3, 30, 301);

//	t1.activate();

	o1.begin(10, 40, 101);
	o2.begin(20, 41, 201);
	o3.begin(30, 42, 301);

//	o3.activate();

	s1.begin(100, 30, 00);
	s2.begin(200, 31, HIGH);
	s3.begin(300, 32, LOW);

#ifdef VISUALSTUDIO
	_pinName(A0, 0, "OK");
	_pinName(A1, 0, "F0");
	_pinName(A2, 0, "F1");
	_pinName(A3, 0, "CANC");
	_pinName(A4, 0, "STOP");
	_pinName(A5, 0, "DCC");
	_pinName(20, 0, "T1");
	_pinName(21, 0, "T2");
	_pinName(22, 0, "T3");
	_pinName(40, 0, "O1");
	_pinName(41, 0, "O2");
	_pinName(42, 0, "O3");
	_pinName(30, 0, "S1");
	_pinName(31, 0, "S2");
	_pinName(32, 0, "S3");
#endif

	Turnout::show();
	Output::show();
	Sensor::show();

	DCCpp::begin();
	DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 11, A6);    // Dc: Dir, Pwm, current sensor

	Turnout::show();
	Output::show();
	Sensor::show();

	locoId = 3;
	locoStepsNumber = 128;
	locoSpeed = 0;
	locoDirectionForward = false;
	//locoFunctions.Clear();	// Already done by the constructor...
}

void loop()
{
	DCCpp::loop();

	char* buffer = externalCommand.received();
	if (buffer != NULL)
		Serial.println(buffer);

	unsigned long event = Commanders::loop();
	int s = 0;

	if (event == UNDEFINED_ID)
		return;

	switch (event)
	{
	case EVENT_MORE:
		if (locoStepsNumber >= 100)
			locoSpeed += 10;
		else
			locoSpeed++;
		if (locoSpeed > locoStepsNumber)
			locoSpeed = locoStepsNumber;
		DCCpp::setSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionForward);
		break;

	case EVENT_LESS:
		if (locoStepsNumber >= 100)
			locoSpeed -= 10;
		else
			locoSpeed--;
		if (locoSpeed < 0)
			locoSpeed = 0;
		DCCpp::setSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionForward);
		break;

	case EVENT_FUNCTION0:
		if (locoFunctions.isActivated(0))
			locoFunctions.inactivate(0);
		else
			locoFunctions.activate(0);
		DCCpp::setFunctionsMain(2, locoId, locoFunctions);
		break;

	case EVENT_FUNCTION1:
		if (locoFunctions.isActivated(1))
			locoFunctions.inactivate(1);
		else
			locoFunctions.activate(1);
		DCCpp::setFunctionsMain(3, locoId, locoFunctions);
		break;

	case EVENT_TURNOUT1:
		if (Commanders::GetLastEventData() == COMMANDERS_MOVE_ON)
			s = 1;
		t1.activate(s);
		break;

	case EVENT_TURNOUT2:
		if (Commanders::GetLastEventData() == COMMANDERS_MOVE_ON)
			s = 1;
		t2.activate(s);
		break;

	case EVENT_TURNOUT3:
		if (Commanders::GetLastEventData() == COMMANDERS_MOVE_ON)
			s = 1;
		t3.activate(s);
		break;
	}
}


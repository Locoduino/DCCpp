/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Minimalist Dcc Controller sample>
*************************************************************/

#include "Commanders.h"

#include "DCCpp.h"

#define EVENT_NONE    0
#define EVENT_MORE    1
#define EVENT_LESS    2
#define EVENT_SELECT  3
#define EVENT_CANCEL  4
#define EVENT_MOVE    5
#define EVENT_START   6
#define EVENT_END   7
#define EVENT_EMERGENCY 8
#define EVENT_FUNCTION0 9
#define EVENT_FUNCTION1 10
#define EVENT_ENCODER 11

ButtonsCommanderPush buttonSelect;
ButtonsCommanderEncoder buttonEncoder;
ButtonsCommanderPush buttonCancel;
ButtonsCommanderPush buttonEmergency;
ButtonsCommanderSwitchOnePin buttonF0;
ButtonsCommanderSwitchOnePin buttonF1;

// in this sample, only one loco is driven...
int locoId; // DCC id for this loco
int locoStepsNumber;  // 14, 28 or 128
int locoSpeed;  // Current speed
bool locoDirectionForward;  // current direction.
FunctionsState locoFunctions; // Current functions

void setup()
{
  Serial.begin(115200);

  buttonSelect.begin(EVENT_SELECT, A0);
  buttonEncoder.begin(EVENT_ENCODER, 14, 8, 2);
  buttonCancel.begin(EVENT_CANCEL, A3);
  buttonEmergency.begin(EVENT_EMERGENCY, A4);
#if defined(ARDUINO_ARCH_ESP32)
  buttonF0.begin(EVENT_FUNCTION0, A5);
  buttonF1.begin(EVENT_FUNCTION1, A6);
#else
  buttonF0.begin(EVENT_FUNCTION0, A1);
  buttonF1.begin(EVENT_FUNCTION1, A2);
#endif

  DCCpp::begin();
  // Configuration for my LMD18200. See the page 'Configuration lines' in the documentation for other samples.
#if defined(ARDUINO_ARCH_ESP32)
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);
#else
  DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 11, A5);
#endif

  locoId = 3;
  locoStepsNumber = 128;
  locoSpeed = 0;
  locoDirectionForward = true;
  //locoFunctions.Clear();  // Already done by the constructor...
}

void loop()
{
  DCCpp::loop();
  unsigned long event = Commanders::loop();

  switch (event)
  {
  case EVENT_ENCODER:
  {
    int data = Commanders::GetLastEventData();

    if (data > 0)
    {
      if (locoStepsNumber >= 100)
        locoSpeed += 10;
      else
        locoSpeed++;
      if (locoSpeed > locoStepsNumber)
        locoSpeed = locoStepsNumber;
      DCCpp::setSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionForward);
    }
    if (data < 0)
    {
      if (locoStepsNumber >= 100)
        locoSpeed -= 10;
      else
        locoSpeed--;
      if (locoSpeed < 0)
        locoSpeed = 0;
      DCCpp::setSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionForward);
    }
    break;
  }

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
    DCCpp::setFunctionsMain(2, locoId, locoFunctions);
    break;
  }
}

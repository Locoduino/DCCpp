/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc Automatic Controller sample>
*************************************************************/

/**
* The goal of this sample is to check if the DCC is working.
*
* The main loop function loops through the StateMachineItems linked list and
* send a command for a given delay. When the delay is expired, it pass to the
* next item. If no command is given, the delay is still taken.
* For instance to let the locomotive stop smoothly...
*
* This sample is made for a DCC address 3, a speed of 50 from 128 during rolling,
* and delays according to my test loco. Your machine could have different reactions...
*/

#include "DCCpp.h"
#include "string.h"

#ifndef USE_TEXTCOMMAND
#error To be able to compile this sample,the line #define USE_TEXTCOMMAND must be uncommented in DCCpp.h
#endif

struct StateMachineItem
{
	static StateMachineItem *first;

	int delay;			// delay for 'this' command
	char command[20];
	char comment[20];
	StateMachineItem *next;

	static StateMachineItem *GetLastItem()
	{
		StateMachineItem *curr = first;
		while (curr != NULL && curr->next != NULL)
			curr = curr->next;

		return curr;
	}

	StateMachineItem(int inDelay, const char *inCommand, const char *inComment)
	{
		this->delay = inDelay;
		strcpy(this->command, inCommand);
		strcpy(this->comment, inComment);
		this->next = NULL;
		if (first == NULL)
			first = this;
		else
			GetLastItem()->next = this;
	}
};

StateMachineItem *StateMachineItem::first = NULL;

StateMachineItem *currentItem = NULL;
unsigned long currentTime = 0;

void setup()
{
	Serial.begin(115200);

	new StateMachineItem(0, "t 1 3 0 1", "ON");				// On register 1, set direction to forward on cab 3
	new StateMachineItem(0, "f 3 144", "Light on");			// Light FL (F0) on
	new StateMachineItem(3000, "t 1 3 50 1", "Forward");	// On register 1, go forward at speed 30 on cab 3
	new StateMachineItem(1000, "t 1 3 0 1", "Stop");		// Stop cab 3 after 1 second
	new StateMachineItem(2000, "", "Wait");					// Wait complete stop
	new StateMachineItem(1000, "t 1 3 0 0", "To bwd");		// On register 1, set direction to backward on cab 3
	new StateMachineItem(3000, "t 1 3 50 0", "Backward");	// On register 1, go backward at speed 30 on cab 3
	new StateMachineItem(1000, "t 1 3 0 0", "Stop");		// Stop cab 3 after 1 second
	new StateMachineItem(2000, "", "Wait");					// Wait complete stop
	new StateMachineItem(1000, "t 1 3 0 1", "To fwd");		// On register 1, set direction to forward on cab 3
	new StateMachineItem(500, "f 3 128", "Light off");		// Light off : blink three times
	new StateMachineItem(500, "f 3 144", "Light on");		// Light FL (F0) on
	new StateMachineItem(500, "f 3 128", "Light off");		// Light off
	new StateMachineItem(500, "f 3 144", "Light on");		// Light on
	new StateMachineItem(500, "f 3 128", "Light off");		// Light off
	new StateMachineItem(500, "f 3 144", "Light on");		// Light on
	new StateMachineItem(500, "f 3 128", "Light off");		// Light off

	DCCpp::begin();
  // Configuration for my LMD18200. See the page 'Configuration lines' in the documentation for other samples.
#if defined(ARDUINO_ARCH_ESP32)
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);
#else
  DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 11, A0);
#endif

	DCCpp::powerOn();

	// Start on first item
	// Start timer too.
	currentItem = StateMachineItem::first;
	currentTime = millis();

	TextCommand::parse(currentItem->command);
}


void loop()
{
	DCCpp::loop();

	if ((int)(millis() - currentTime) >= currentItem->delay)
	{
		currentItem = currentItem->next;
		if (currentItem == NULL)
		{
			currentItem = StateMachineItem::first;
			Serial.println("Restart");
		}

		currentTime = millis();
		TextCommand::parse(currentItem->command);
		Serial.println(currentItem->comment);
	}
}

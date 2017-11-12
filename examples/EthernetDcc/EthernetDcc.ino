/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc Serial Controller sample>
*************************************************************/

#include "DCCpp.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_ETHERNET)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_ETHERNET must be uncommented in DCCpp.h
#endif

#include <UIPEthernet.h>

// the media access control (ethernet hardware) address for the shield:
uint8_t mac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0xEF };
//the IP address for the shield:
uint8_t ip[] = { 192, 168, 1, 200 };

EthernetServer INTERFACE(2560);                  // Create and instance of an EnternetServer

void setup()
{
	Serial.begin(115200);

	DCCpp.begin();
	DCCpp.beginMain(255, 12, 50, A5);
	DCCpp.beginEthernet(mac, ip);
}

void loop()
{
	DCCpp.loop();
}


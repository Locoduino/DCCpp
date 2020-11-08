/*************************************************************
project: <Dc/Dcc Controller>
author: <Thierry PARIS>
description: <Dcc Ethernet Controller sample>
*************************************************************/

#include "DCCpp.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_ETHERNET)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_ETHERNET must be uncommented in DCCpp.h
#endif

// the media access control (ethernet hardware) address for the shield:
uint8_t mac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0xEF };
//the IP address for the shield:
uint8_t ip[] = { 192, 168, 1, 200 };

EthernetServer DCCPP_INTERFACE(2560);                  // Create and instance of an EthernetServer

void setup()
{
	Serial.begin(115200);

	DCCpp::begin();
  // Configuration for my LMD18200. See the page 'Configuration lines' in the documentation for other samples.
#if defined(ARDUINO_ARCH_ESP32)
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);
#else
	DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);
#endif
	DCCpp::beginEthernet(mac, ip, EthernetProtocol::TCP);
}

void loop()
{
	DCCpp::loop();
}


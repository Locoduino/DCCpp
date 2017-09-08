/**********************************************************************

Comm.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Config.h"

#if USE_ETHERNET                 // Ethernet Shield Card Selected

  #if USE_ETHERNET_ARDUINO_CC == 1
    #define COMM_SHIELD_NAME      "ARDUINO-CC ETHERNET SHIELD (WIZNET 5100)"
    #include <Ethernet.h>         // built-in Arduino.cc library

  #elif USE_ETHERNET_ARDUINO_ORG == 2
    #define COMM_SHIELD_NAME      "ARDUINO-ORG ETHERNET-2 SHIELD (WIZNET 5500)"
    #include <Ethernet2.h>        // https://github.com/arduino-org/Arduino

  #elif USE_ETHERNET_SEEED == 3
    #define COMM_SHIELD_NAME      "SEEED STUDIO ETHERNET SHIELD (WIZNET 5200)"
    #include <EthernetV2_0.h>     // https://github.com/Seeed-Studio/Ethernet_Shield_W5200

  #endif

  extern EthernetServer INTERFACE;
#endif  





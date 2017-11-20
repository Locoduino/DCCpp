//-------------------------------------------------------------------
#ifndef __DCCpp_H__
#define __DCCpp_H__
//-------------------------------------------------------------------

/**	 @mainpage
**********************************************************************

DCC++ BASE STATION
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Adapted in the library DCCpp by Thierry PARIS
Copyright Locoduino 2017

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses

**********************************************************************
      
DCC++ BASE STATION is a C++ program written for the Arduino Uno and Arduino Mega
using the Arduino IDE 1.6.6.

It allows a standard Arduino Uno or Mega with an Arduino Motor Shield (as well as others)
to be used as a fully-functioning digital command and control (DCC) base station
for controlling model train layouts that conform to current National Model
Railroad Association (NMRA) DCC standards.

This version of DCC++ BASE STATION supports:

  * 2-byte and 4-byte locomotive addressing
  * Simultaneous control of multiple locomotives
  * 128-step speed throttling
  * Cab functions F0-F28
  * Activate/de-activate accessory functions using 512 addresses, each with 4 sub-addresses
      - includes optional functionailty to monitor and store of the direction of any connected turnouts
  * Programming on the Main Operations Track
      - write configuration variable bytes
      - set/clear specific configuration variable bits
  * Programming on the Programming Track
      - write configuration variable bytes
      - set/clear specific configuration variable bits
      - read configuration variable bytes

DCC++ BASE STATION is controlled with simple text commands received via
the Arduino's serial interface.  Users can type these commands directly
into the Arduino IDE Serial Monitor, or can send such commands from another
device or computer program.

When compiled for the Arduino Mega, an Ethernet Shield can be used for network
communications instead of using serial communications.

DCC++ CONTROLLER, available separately under a similar open-source
license, is a Java program written using the Processing library and Processing IDE
that provides a complete and configurable graphic interface to control model train layouts
via the DCC++ BASE STATION.

With the exception of a standard 15V power supply that can be purchased in
any electronics store, no additional hardware is required.

Neither DCC++ BASE STATION nor DCC++ CONTROLLER use any known proprietary or
commercial hardware, software, interfaces, specifications, or methods related
to the control of model trains using NMRA DCC standards.  Both programs are wholly
original, developed by the author, and are not derived from any known commercial,
free, or open-source model railroad control packages by any other parties.

However, DCC++ BASE STATION and DCC++ CONTROLLER do heavily rely on the IDEs and
embedded libraries associated with Arduino and Processing.  Tremendous thanks to those
responsible for these terrific open-source initiatives that enable programs like
DCC++ to be developed and distributed in the same fashion.

REFERENCES:

  NMRA DCC Standards:          http://www.nmra.org/index-nmra-standards-and-recommended-practices
  Arduino:                     http://www.arduino.cc/
  Processing:                  http://processing.org/
  GNU General Public License:  http://opensource.org/licenses/GPL-3.0

BRIEF NOTES ON THE THEORY AND OPERATION OF DCC++ BASE STATION:

DCC++ BASE STATION for the Uno configures the OC0B interrupt pin associated with Timer 0,
and the OC1B interupt pin associated with Timer 1, to generate separate 0-5V
unipolar signals that each properly encode zero and one bits conforming with
DCC timing standards.  When compiled for the Mega, DCC++ BASE STATION uses OC3B instead of OC0B.

Series of DCC bit streams are bundled into Packets that each form the basis of
a standard DCC instruction.  Packets are stored in Packet Registers that contain
methods for updating and queuing according to text commands sent by the user
(or another program) over the serial interface.  There is one set of registers that controls
the main operations track and one that controls the programming track.

For the main operations track, packets to store cab throttle settings are stored in
registers numbered 1 through MAX_MAIN_REGISTERS (as defined in DCCpp_Uno.h).
It is generally considered good practice to continuously send throttle control packets
to every cab so that if an engine should momentarily lose electrical connectivity with the tracks,
it will very quickly receive another throttle control signal as soon as connectivity is
restored (such as when a trin passes over  rough portion of track or the frog of a turnout).

DCC++ Base Station therefore sequentially loops through each main operations track packet regsiter
that has been loaded with a throttle control setting for a given cab.  For each register, it
transmits the appropriate DCC packet bits to the track, then moves onto the next register
without any pausing to ensure continuous bi-polar power is being provided to the tracks.
Updates to the throttle setting stored in any given packet register are done in a double-buffered
fashion and the sequencer is pointed to that register immediately after being changes so that updated DCC bits
can be transmitted to the appropriate cab without delay or any interruption in the bi-polar power signal.
The cabs identified in each stored throttle setting should be unique across registers.  If two registers
contain throttle setting for the same cab, the throttle in the engine will oscillate between the two,
which is probably not a desireable outcome.

For both the main operations track and the programming track there is also a special packet register with id=0
that is used to store all other DCC packets that do not require continious transmittal to the tracks.
This includes DCC packets to control decoder functions, set accessory decoders, and read and write Configuration Variables.
It is common practice that transmittal of these one-time packets is usually repeated a few times to ensure
proper receipt by the receiving decoder.  DCC decoders are designed to listen for repeats of the same packet
and provided there are no other packets received in between the repeats, the DCC decoder will not repeat the action itself.
Some DCC decoders actually require receipt of sequential multiple identical one-time packets as a way of
verifying proper transmittal before acting on the instructions contained in those packets

An Arduino Motor Shield (or similar), powered by a standard 15V DC power supply and attached
on top of the Arduino Uno or Mega, is used to transform the 0-5V DCC logic signals
produced by the Uno's Timer interrupts into proper 0-15V bi-polar DCC signals.

This is accomplished on the Uno by using one small jumper wire to connect the Uno's OC1B output (pin 10)
to the Motor Shield's DIRECTION A input (pin 12), and another small jumper wire to connect
the Uno's OC0B output (pin 5) to the Motor Shield's DIRECTION B input (pin 13).

For the Mega, the OC1B output is produced directly on pin 12, so no jumper is needed to connect to the
Motor Shield's DIRECTION A input.  However, one small jumper wire is needed to connect the Mega's OC3B output (pin 2)
to the Motor Shield's DIRECTION B input (pin 13).

Other Motor Shields may require different sets of jumper or configurations (see Config.h and DCCpp_Uno.h for details).

When configured as such, the CHANNEL A and CHANNEL B outputs of the Motor Shield may be
connected directly to the tracks.  This software assumes CHANNEL A is connected
to the Main Operations Track, and CHANNEL B is connected to the Programming Track.

DCC++ BASE STATION in split into multiple modules, each with its own header file:

  DCCpp_Uno:        declares required global objects and contains initial Arduino setup()
                    and Arduino loop() functions, as well as interrput code for OC0B and OC1B.
                    Also includes declarations of optional array of Turn-Outs and optional array of Sensors 

  TextCommand:      contains methods to read and interpret text commands from the serial line,
                    process those instructions, and, if necessary call appropriate Packet RegisterList methods
                    to update either the Main Track or Programming Track Packet Registers. (old name SerialCommand)

  PacketRegister:   contains methods to load, store, and update Packet Registers with DCC instructions

  CurrentMonitor:   contains methods to separately monitor and report the current drawn from CHANNEL A and
                    CHANNEL B of the Arduino Motor Shield's, and shut down power if a short-circuit overload
                    is detected

  Turnout:          contains methods to operate and store the status of any optionally-defined turnouts controlled
                    by a DCC stationary accessory decoder. (old name Accessories)

  Sensor:           contains methods to monitor and report on the status of optionally-defined infrared
                    sensors embedded in the Main Track and connected to various pins on the Arudino Uno

  Output:           contains methods to configure one or more Arduino pins as an output for your own custom use

  EEStore:          contains methods to store, update, and load various DCC settings and status
                    (e.g. the states of all defined turnouts) in the EEPROM for recall after power-up

DCC++ BASE STATION is configured through the Config.h file that contains all user-definable parameters 

*************************************************************************************************

Complement of the documentation for this library:

This library is a free interpretation of the Gregg's work to adapt it in a library, and minimize
the needs to modify the library sources to use it.

\page Revision History

\par 25/08/2017 V0 Initial Release
- DCCpp est le portage du programme DCC++ en bibliothèque.
_______________
- DCCpp is the transcription of the DCC++ program into a library.
*/

/** @file DCCpp.h
Main include file of the library.*/

////////////////////////////////////////////////////////
// Add a '//' at the beginning of the line to be in release mode.
//#define DCCPP_DEBUG_MODE

///////////////////////////////////////////////////////
// Verbose mode lets you see all actions done by the 
// library, but with a real flood of text to console...
// Has no effect if DCCPP_DEBUG_MODE is not activated.
//#define DCCPP_DEBUG_VERBOSE_MODE

///////////////////////////////////////////////////////
// The function DCCpp::printConfiguration()
// is very heavy in program memory. So to avoid problems
// you can make this function available by uncomment the next line.
//#define DCCPP_PRINT_DCCPP

//  Inclusion area
//

/**Comment this line to avoid using and compiling Turnout.*/
//#define USE_TURNOUT
/**Comment this line to avoid using and compiling EEPROM saving.*/
//#define USE_EEPROM
/**Comment this line to avoid using and compiling Outputs.*/
//#define USE_OUTPUT
/**Comment this line to avoid using and compiling Sensors.*/
//#define USE_SENSOR
/**Comment this line to avoid using and compiling Serial commands.*/
//#define USE_TEXTCOMMAND
/**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5100 chip.*/
//#define USE_ETHERNET_WIZNET_5100
/**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5500 chip.*/
//#define USE_ETHERNET_WIZNET_5500
/**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5200 chip.*/
//#define USE_ETHERNET_WIZNET_5200
/**Comment this line to avoid using and compiling Ethernet shield using ENC28J60 chip.*/
//#define USE_ETHERNET_ENC28J60

#if defined(USE_ETHERNET_WIZNET_5100) || defined(USE_ETHERNET_WIZNET_5500) || defined(USE_ETHERNET_WIZNET_5200) || defined(USE_ETHERNET_ENC28J60)
#define USE_ETHERNET
#endif

/////////////////////////////////////

#include "DCCpp_Uno.h"
#include "PacketRegister.h"
#include "CurrentMonitor.h"
#include "Config.h"
#include "Comm.h"

#ifdef USE_TURNOUT
#include "Turnout.h"
#endif
#ifdef USE_EEPROM
#include "EEStore.h"
#endif
#ifdef USE_OUTPUT
#include "Outputs.h"
#endif
#ifdef USE_SENSOR
#include "Sensor.h"
#endif
#ifdef USE_TEXTCOMMAND
#include "TextCommand.h"
#endif

#include "DCCpp.hpp"

#ifdef DOXYGEN_SPECIFIC
// DO NOT CHANGE THE LINES IN THIS BLOCK 'DOXYGEN_SPECIFIC' : Only here for documentation !

/** If this is defined, the library will do many checks during setup and execution, and print errors, warnings and
information messages on console. These messages can take a lot of memory, so be careful about the free memory of
your program if you activate debug mode.*/
#define DCCPP_DEBUG_MODE
/** If this is defined, the Verbose mode lets you see all actions done by the  library, but with a real flood of
text to the console... It has no effect if DCCPP_DEBUG_MODE is not activated.*/
#define DCCPP_DEBUG_VERBOSE_MODE
/** If this is defined, the function Accessories::printAccessories() will become available. This is useful to try
to understand why a port, or an accessory is not corretly defined.
This function uses a lot of memory, so activate it only if necessary, and be careful about your program's memory.
You can use the define PRINT_DCCPP() in your sketch instead of a call to DCCpp.showConfiguration().
If DCCPP_PRINT_DCCPP is not defined, PRINT_DCCPP is defined as empty, so you will not have a compilation error.*/
#define DCCPP_PRINT_DCCPP
/** If this is defined, the state of all the library will not be saved to EEPROM.*/
#endif

#endif
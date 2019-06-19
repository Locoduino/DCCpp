//-------------------------------------------------------------------
#ifndef __DCCpp_H__
#define __DCCpp_H__
//-------------------------------------------------------------------

/**	 @mainpage
-------------------------------------------------------------------------------------------------------

DCC++ BASE STATION
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Adapted in this library DCCpp by Thierry PARIS
Copyright Locoduino 2017-2019

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

-------------------------------------------------------------------------------------------------------
Original description of DCC++from Gregg, see below for complements for this library :

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
  * Activate/deactivate accessory functions using 512 addresses, each with 4 sub-addresses
      - includes optional functionality to monitor and store of the direction of any connected turnouts
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
device or computer program.	A DCCpp class can also send orders through methods without any text.

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

  - NMRA DCC Standards:          http://www.nmra.org/index-nmra-standards-and-recommended-practices
  - Arduino:                     http://www.arduino.cc/
  - Processing:                  http://processing.org/
  - GNU General Public License:  http://opensource.org/licenses/GPL-3.0

BRIEF NOTES ON THE THEORY AND OPERATION OF DCC++ BASE STATION:

DCC++ BASE STATION for the Uno configures the OC0B interrupt pin associated with Timer 0,
and the OC1B interrupt pin associated with Timer 1, to generate separate 0-5V
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
restored (such as when a train passes over  rough portion of track or the frog of a turnout).

DCC++ Base Station therefore sequentially loops through each main operations track packet register
that has been loaded with a throttle control setting for a given cab.  For each register, it
transmits the appropriate DCC packet bits to the track, then moves onto the next register
without any pausing to ensure continuous bi-polar power is being provided to the tracks.
Updates to the throttle setting stored in any given packet register are done in a double-buffered
fashion and the sequencer is pointed to that register immediately after being changes so that updated DCC bits
can be transmitted to the appropriate cab without delay or any interruption in the bi-polar power signal.
The cabs identified in each stored throttle setting should be unique across registers.  If two registers
contain throttle setting for the same cab, the throttle in the engine will oscillate between the two,
which is probably not a desirable outcome.

For both the main operations track and the programming track there is also a special packet register with id=0
that is used to store all other DCC packets that do not require continuous transmittal to the tracks.
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

Other Motor Shields may require different sets of jumper or configurations (see \ref commonPage page for other details).

When configured as such, the CHANNEL A and CHANNEL B outputs of the Motor Shield may be
connected directly to the tracks.  This software assumes CHANNEL A is connected
to the Main Operations Track, and CHANNEL B is connected to the Programming Track.

DCC++ BASE STATION in split into multiple modules, each with its own header file:

  DCCpp_Uno:        declares required global objects and contains initial Arduino setup()
                    and Arduino loop() functions, as well as interrupt code for OC0B and OC1B.
                    Also includes declarations of optional array of Turn-Outs and optional array of Sensors. 

  TextCommand:      contains methods to read and interpret text commands from the serial/Ethernet line,
                    process those instructions, and, if necessary call appropriate Packet RegisterList methods
                    to update either the Main Track or Programming Track Packet Registers. (old name SerialCommand).

  PacketRegister:   contains methods to load, store, and update Packet Registers with DCC instructions.

  CurrentMonitor:   contains methods to separately monitor and report the current drawn from CHANNEL A and
                    CHANNEL B of the Arduino Motor Shield's, and shut down power if a short-circuit overload
                    is detected.

  Turnout:          contains methods to operate and store the status of any optionally-defined turnouts controlled
                    by a DCC stationary accessory decoder. (old name Accessories).

  Sensor:           contains methods to monitor and report on the status of optionally-defined infrared
                    sensors embedded in the Main Track and connected to various pins on the Arduino Uno.

  Output:           contains methods to configure one or more Arduino pins as an output for your own custom use.

  EEStore:          contains methods to store, update, and load various DCC settings and status
                    (e.g. the states of all defined turnouts) in the EEPROM for recall after power-up.

-------------------------------------------------------------------------------------------------------

Complement of the documentation for this library:

This library is a free interpretation of the Gregg's work to adapt it to a library, and minimize
the needs to modify the library sources to use it. The only configuration still needed in .h file is 
to decide of the Ethernet interface model.
It has been adapted to work also with Arduino Nano R3 on IDE 1.8.4 .

\page commonPage Common Configuration Lines

This is the 'begin' lines for some common configurations. Note that for LMD18200, the two final arguments must be adapted to your need...
The wiring for these configurations is visible here : http://www.locoduino.org/spip.php?article187 . The text is in French, but schema can be understood !

For Arduino or Pololu shields, signalPinMain must be connected to Direction motor A, and signalPinProg to Direction motor B
If a track is not connected, main or programming, the signalPin should stay to default at 255 (UNDEFINED_PIN).
For H bridge connected directly to the pins, like LMD18200, signalPin and Direction motor should have the same pin number,
or directions can be set to UNDEFINED_PIN.

\par Arduino Uno + LMD18200 + MAX471

\verbatim
DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);
\endverbatim

\par Arduino Uno + 2 LMD18200 + 2 MAX471

\verbatim
DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);
DCCpp::beginProg(UNDEFINED_PIN, DCC_SIGNAL_PIN_PROG, 5, A1);
\endverbatim

\par Arduino Mega2560 + LMD18200 + MAX471

\verbatim
DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);
\endverbatim

\par Arduino Mega2560 + 2 LMD18200 + 2 MAX471

\verbatim
DCCpp::beginMain(UNDEFINED_PIN, DCC_SIGNAL_PIN_MAIN, 3, A0);
DCCpp::beginProg(UNDEFINED_PIN, DCC_SIGNAL_PIN_PROG, 11, A1);
\endverbatim

\par Arduino Uno or Mega2560 + Arduino Motor Shield

\verbatim
DCCpp::beginMainMotorShield();
DCCpp::beginProgMotorShield();
\endverbatim
or
\verbatim
DCCpp::beginMain(MOTOR_SHIELD_DIRECTION_MOTOR_CHANNEL_PIN_A, DCC_SIGNAL_PIN_MAIN, MOTOR_SHIELD_SIGNAL_ENABLE_PIN_MAIN, MOTOR_SHIELD_CURRENT_MONITOR_PIN_MAIN);
DCCpp::beginProg(MOTOR_SHIELD_DIRECTION_MOTOR_CHANNEL_PIN_B, DCC_SIGNAL_PIN_PROG, MOTOR_SHIELD_SIGNAL_ENABLE_PIN_PROG, MOTOR_SHIELD_CURRENT_MONITOR_PIN_PROG);
\endverbatim

\par Arduino Uno or Mega2560 + Pololu MC33926 Motor Shield

\verbatim
DCCpp::beginMainPololu();
DCCpp::beginProgPololu();
\endverbatim
or
\verbatim
DCCpp::beginMain(POLOLU_DIRECTION_MOTOR_CHANNEL_PIN_A, DCC_SIGNAL_PIN_MAIN, POLOLU_SIGNAL_ENABLE_PIN_MAIN, POLOLU_CURRENT_MONITOR_PIN_MAIN);
DCCpp::beginProg(POLOLU_DIRECTION_MOTOR_CHANNEL_PIN_B, DCC_SIGNAL_PIN_PROG, POLOLU_SIGNAL_ENABLE_PIN_PROG, POLOLU_CURRENT_MONITOR_PIN_PROG);
\endverbatim

\page revPage Revision History
\par 19/06/2019 V1.3.5
- Correction de la transformation de INTERFACE en DCCPP_INTERFACE si USE_ETHERNET_* .
_______________
- Fix INTERFACE to DCCPP_INTERFACE if compiled with USE_ETHERNET_*.

\par 10/06/2019 V1.3.4
- Corrections d'INTERFACE en DCCPP_INTERFACE sur des parties non copilées par défaut qui avaient échappés à la tranformaition.
- CurrentMonitor permet de mieux gérer les surcharges séparément entre la voie principale et la voie de programmation.
_______________
- Fix INTERFACE - DCCPP_INTERFACE mistakes on hiden parts of the code not always compiled...
- CurrentMonitor can now handle separatly overloads on the main track and the prog track.

\par 03/06/2019 V1.3.3
- Mise à jour de la partie Visual Studio pour le nouveau ArduiEmulatorWin
- Remplacement de INTERFACE par DCCPP_INTERFACE pour éviter un conflit de nommage
- Change LIBRARY_VERSION en DCCPP_LIBRARY_VERSION
- Correction de l'exemple EthernetDcc pour le numéro de pin à 11 au lieu de 3...
- Retrait d'un fichier inutile dans les sources.
- powerOn() et powerOff() peuvent maintenant couper/remettre le courant sur les deux voies indépendamment.
_______________
- Update to use the new ArduiEmulatorWin
- Replace of INTERFACE by DCCPP_INTERFACE to avoid naming conflict.
- Change LIBRARY_VERSION to DCCPP_LIBRARY_VERSION
- Fix pin number from 11 to 3 in sample EthernetDcc.ino
- Removing of an unused file in src.
- powerOn() and powerOff() can now stop/restore the current on each track independantly.

\par 23/12/2018 V1.3.2
- Correction de beginProgMotorShield() qui appelait beginMain au lieu de beginProg
- Correction de beginProgPololu() qui appelait beginMain au lieu de beginProg
_______________
- definition of beginProgMotorShield() call now beginProg instead of beginMain
- definition of beginProgPololu() call now beginProg instead of beginMain

\par 15/07/2018 V1.3.1
- Correction du format de Keywords.txt
- Création de LIBRARY_VERSION
- Mise en conformité de la partie doc HTML avec DOXYGEN_SPECIFIC
- Ajout du fichier LICENSE
_______________
- Fix Keywords.txt format.
- LIBRARY_VERSION created
- Update of DOXYGEN_SPECIFIC part for HTML documentation.
- Add of LICENSE file

\par 02/04/2018 V1.3.0
- Ajout de IsMainTrack qui identifie la voie (principale/programmation) selon le RegisterList en argument.
- Correction de identifyLocoId qui ne marchait que sur la voie principale.
- Correction de arduino.h en Arduino.h
_______________
- Add of IsMainTrack to identify the track (main/prog) according to the given RegisterList.
- Fix identifyLocoId which was only working on main track.
- Change arduino.h in Arduino.h

\par 08/01/2018 V1.2.0
- Retrait de l'argument locoId des fonctions de lecture/écriture de CV : sans objet !
- Ajout des arguments callback et callbacksub de passage d'information libre dans les fonctions readCv et writeCv. C'est une mise en conformité avec les fonctions originales.
- Amélioration des fonctions identifyLocoIdMain et Prog pour lire aussi les adresses longues.
_______________
- Argument locoId removed from read and write CV functions : not used !
- Arguments callback and callbacksub added to read/write CVs, to be conform to original functions.
- Improvement of new functions identifyLocoIdMain and Prog to be able to get long addresses.

\par 08/01/2018 V1.1.0
- Correction de readCV(ints) et readCV(string) qui ne retournaient aucune valeur !
- Correction d'un warning dans les exemples miniDcc et maxiDcc.
- Ajout de identifyLocoIdMain() et identifyLocoIdProg() pour lire le CV1 d'une loco présente sur la voie.
_______________
- Fix readCV(ints) and readCV(string) to return values !
- Fix a warning in samples mini and maxi.
- Added identifyLocoIdMain() and identifyLocoIdProg() to find the address of a decoder on the track.

\par 08/01/2018 V1.0.1
- Correction des exemples mini et maxi qui ne géraient pas correctement l'encodeur.
- Ajout d'un test pour un beginMain/beginProg avec le signal pin à 255.
_______________
- Fix samples mini and maxi which did not use EVENT_ENCODER event !
- Add a test in beginMain/beginProg to check undefined signal pin.

\par 27/12/2017 V1.0.0
- Pour se conformer à la norme DCC, la commande DCC de fonction est doublée.
- Correction de Sensor dont le 'check' provoquait une grande quantité de lignes vides sur la console.
_______________
- To be closer of the DCC norm, the function DCC orders will be send twice.
- Fix Sensor::check(). A lot of empty lines was sent to the Serial console...

\par 22/12/2017 V0.9.0
- Envoi de la seule commande DCC concernée lors d'une activation/désactivation de fonction.
- Nouvel exemple Autotest.
_______________
- Send only necessary DCC command when activating/dis-activating a function.
- New sample Autotest.

\par 11/12/2017 V0.8.0
- fin de la documentation
- DCCpp classe passée en statique
- Ajout d'un test de numéro de registre sur setFunctions()
- les variables 'ToLeft' ont été changées pour 'Forward', pour mieux coller à la norme DCC.
- Correction de EEStore.store()
_______________
- End of documentation
- DCCpp class goes to static.
- setFunctions() now checks for register number in debug mode.
- 'ToLeft' variables have been changed to 'Forward', closer from DCC norm.
- Fix EEStore.store()

\par 27/11/2017 V0.7.0
- Retour des chaines de validation de commande renvoyées à l'interface...
- Définition de ARDUINO_AVR_MEGA2560 si ARDUINO_AVR_MEGA défini.
- Ajout des fonctions beginMainMotorShield, beginProgMotorShield, beginMainPololu et beginProgPololu
- EthernetProtocol::None supprimé.
_______________
- Acknowledgment strings are back.
- ARDUINO_AVR_MEGA2560 defined if ARDUINO_AVR_MEGA defined.
- Added functions beginMainMotorShield, beginProgMotorShield, beginMainPololu and beginProgPololu
- EthernetProtocol::None removed.

\par 25/11/2017 V0.6.0
- Retour des chaines de validation de commande renvoyées à l'interface...
- Corrections de Turnout et Output quand EEPROM utilisée.
_______________
- Acknowledgment strings are back.
- Fixes for Turnout and Output when EEPROM is used.

\par 25/08/2017 V0 Initial Release
- DCCpp est le portage du programme DCC++ en bibliothèque.
_______________
- DCCpp is the transcription of the DCC++ program into a library.
*/

/** @file DCCpp.h
Main include file of the library.*/

#define DCCPP_LIBRARY_VERSION		"VERSION DCCpp library: 1.3.5"

////////////////////////////////////////////////////////
// Add a '//' at the beginning of the line to be in production mode.
//#define DCCPP_DEBUG_MODE

///////////////////////////////////////////////////////
// Verbose mode lets you see all actions done by the 
// library, but with a real flood of text to console...
// Has no effect if DCCPP_DEBUG_MODE is not activated.
//#define DCCPP_DEBUG_VERBOSE_MODE

///////////////////////////////////////////////////////
// The function DCCpp::printConfiguration()
// is very heavy in program memory. So to avoid problems
// you can make this function available by uncomment the next line, only when necessary.
//#define DCCPP_PRINT_DCCPP

//  Inclusion area
//

#define USE_TURNOUT
#define USE_EEPROM
#define USE_OUTPUT
#define USE_SENSOR
#define USE_TEXTCOMMAND
#define USE_ETHERNET_WIZNET_5100
//#define USE_ETHERNET_WIZNET_5500
//#define USE_ETHERNET_WIZNET_5200
//#define USE_ETHERNET_ENC28J60

#ifdef DOXYGEN_SPECIFIC
    // DO NOT CHANGE THESE LINES IN THIS BLOCK 'DOXYGEN_SPECIFIC' : Only here for documentation !

    /**Comment this line to avoid using and compiling Turnout.*/
    #define USE_TURNOUT
    /**Comment this line to avoid using and compiling EEPROM saving.*/
    #define USE_EEPROM
    /**Comment this line to avoid using and compiling Outputs.*/
    #define USE_OUTPUT
    /**Comment this line to avoid using and compiling Sensors.*/
    #define USE_SENSOR
    /**Comment this line to avoid using and compiling Serial commands.*/
    #define USE_TEXTCOMMAND
    /**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5100 chip (Arduino Shield v1).*/
    #define USE_ETHERNET_WIZNET_5100
    /**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5500 chip (Arduino Shield v2).*/
    #define USE_ETHERNET_WIZNET_5500
    /**Comment this line to avoid using and compiling Ethernet shield using Wiznet 5200 chip (Seeed Studio).*/
    #define USE_ETHERNET_WIZNET_5200
    /**Comment this line to avoid using and compiling Ethernet shield using ENC28J60 chip.*/
    #define USE_ETHERNET_ENC28J60

    #undef USE_TURNOUT
    #undef USE_EEPROM
    #undef USE_OUTPUT
    #undef USE_SENSOR
    #undef USE_TEXTCOMMAND
    #undef USE_ETHERNET_WIZNET_5100
    #undef USE_ETHERNET_WIZNET_5500
    #undef USE_ETHERNET_WIZNET_5200
    #undef USE_ETHERNET_ENC28J60

    /** If this is defined, the library will do many checks during setup and execution, and print errors, warnings and
    information messages on console. These messages can take a lot of memory, so be careful about the free memory of
    your program if you activate debug mode.*/
    #define DCCPP_DEBUG_MODE
    /** If this is defined, the Verbose mode lets you see all actions done by the  library, but with a real flood of
    text to the console... It has no effect if DCCPP_DEBUG_MODE is not activated.*/
    #define DCCPP_DEBUG_VERBOSE_MODE
    /** If this is defined, the function DCCpp::showConfiguration() will become available. This is useful to try
    to understand why something is not correctly defined.
    This function uses a lot of memory, so activate it only if necessary, and be careful about your program's memory.
    You can use the define PRINT_DCCPP() in your sketch instead of a call to DCCpp.showConfiguration().
    If DCCPP_PRINT_DCCPP is not defined, PRINT_DCCPP is defined as empty, so you will not have a compilation error.*/
    #define DCCPP_PRINT_DCCPP
#endif

#if defined(USE_ETHERNET_WIZNET_5100) || defined(USE_ETHERNET_WIZNET_5500) || defined(USE_ETHERNET_WIZNET_5200) || defined(USE_ETHERNET_ENC28J60)
#define USE_ETHERNET
#endif

/////////////////////////////////////

#ifdef ARDUINO_ARCH_AVR
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
#endif

#include "DCCpp.hpp"
#endif

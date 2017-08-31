/**********************************************************************

Config.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman
Adapted for DcDcc by Thierry PARIS

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/
#ifndef __config_h
#define __config_h

#include "arduino.h"

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE MOTOR_SHIELD_TYPE ACCORDING TO THE FOLLOWING TABLE:
//
//  0 = ARDUINO MOTOR SHIELD          (MAX 18V/2A PER CHANNEL)
//  1 = POLOLU MC33926 MOTOR SHIELD   (MAX 28V/3A PER CHANNEL)

#define MOTOR_SHIELD_TYPE   0

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE NUMBER OF MAIN TRACK REGISTER

#define MAX_MAIN_REGISTERS 12

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE COMMUNICATIONS INTERFACE
//
//  0 = Built-in Serial Port
//  1 = Arduino.cc Ethernet/SD-Card Shield
//  2 = Arduino.org Ethernet/SD-Card Shield
//  3 = Seeed Studio Ethernet/SD-Card Shield W5200

#define COMM_INTERFACE   0

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE STATIC IP ADDRESS *OR* COMMENT OUT TO USE DHCP
//

//#define IP_ADDRESS { 192, 168, 1, 200 }

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE PORT TO USE FOR ETHERNET COMMUNICATIONS INTERFACE
//

#define ETHERNET_PORT 2560

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE MAC ADDRESS ARRAY FOR ETHERNET COMMUNICATIONS INTERFACE
//

#define MAC_ADDRESS {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF }

/////////////////////////////////////////////////////////////////////////////////////
//
// DEFINE PINS ACCORDING TO MOTOR SHIELD MODEL
//

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO

#define DCC_SIGNAL_PIN_MAIN 10          // Ardunio Uno  - uses OC1B
#define DCC_SIGNAL_PIN_PROG 5           // Arduino Uno  - uses OC0B

#elif defined  ARDUINO_AVR_MEGA

#define DCC_SIGNAL_PIN_MAIN 12          // Arduino Mega - uses OC1B
#define DCC_SIGNAL_PIN_PROG 2           // Arduino Mega - uses OC3B

#endif

/////////////////////////////////////////////////////////////////////////////////////
// SELECT MOTOR SHIELD
/////////////////////////////////////////////////////////////////////////////////////

#define MOTOR_SHIELD_SIGNAL_ENABLE_PIN_MAIN 3
#define MOTOR_SHIELD_SIGNAL_ENABLE_PIN_PROG 11

#define MOTOR_SHIELD_CURRENT_MONITOR_PIN_MAIN A0
#define MOTOR_SHIELD_CURRENT_MONITOR_PIN_PROG A1

#define MOTOR_SHIELD_DIRECTION_MOTOR_CHANNEL_PIN_A 12
#define MOTOR_SHIELD_DIRECTION_MOTOR_CHANNEL_PIN_B 13

#define POLULU_SIGNAL_ENABLE_PIN_MAIN 9
#define POLULU_SIGNAL_ENABLE_PIN_PROG 11

#define POLULU_CURRENT_MONITOR_PIN_MAIN A0
#define POLULU_CURRENT_MONITOR_PIN_PROG A1

#define POLULU_DIRECTION_MOTOR_CHANNEL_PIN_A 7
#define POLULU_DIRECTION_MOTOR_CHANNEL_PIN_B 8

struct DCCppConfig
{
	static byte SignalEnablePinMain;// PWM : *_SIGNAL_ENABLE_PIN_MAIN
	static byte CurrentMonitorMain;	// Current sensor : *_CURRENT_MONITOR_PIN_MAIN

	static byte SignalEnablePinProg;
	static byte CurrentMonitorProg;

	// Only for shields : indirection of the signal from SignalPinMain to DirectionMotor of the shield
	static byte DirectionMotorA;	// *_DIRECTION_MOTOR_CHANNEL_PIN_A
	static byte DirectionMotorB;	// *_DIRECTION_MOTOR_CHANNEL_PIN_B
};

/////////////////////////////////////////////////////////////////////////////////////

#endif
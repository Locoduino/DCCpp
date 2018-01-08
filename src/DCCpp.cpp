/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <DCCpp class>
*************************************************************/

#include "DCCpp.h"
#include "arduino.h"

// NEXT DECLARE GLOBAL OBJECTS TO PROCESS AND STORE DCC PACKETS AND MONITOR TRACK CURRENTS.
// NOTE REGISTER LISTS MUST BE DECLARED WITH "VOLATILE" QUALIFIER TO ENSURE THEY ARE PROPERLY UPDATED BY INTERRUPT ROUTINES

volatile RegisterList DCCpp::mainRegs(MAX_MAIN_REGISTERS);    // create list of registers for MAX_MAIN_REGISTER Main Track Packets
volatile RegisterList DCCpp::progRegs(3);                     // create a shorter list of only two registers for Program Track Packets

CurrentMonitor DCCpp::mainMonitor;  // create monitor for current on Main Track
CurrentMonitor DCCpp::progMonitor;  // create monitor for current on Program Track

bool DCCpp::programMode;
bool DCCpp::panicStopped; 

// *********************************************************** FunctionsState

FunctionsState::FunctionsState()
{
	this->clear();
}

void FunctionsState::clear()
{
	// Clear all functions
	this->activeFlags[0] = 0;
	this->activeFlags[1] = 0;
	this->activeFlags[2] = 0;
	this->activeFlags[3] = 0;

	this->statesSent();
}

void FunctionsState::activate(byte inFunctionNumber)
{
	bitSet(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

void FunctionsState::inactivate(byte inFunctionNumber)
{
	bitClear(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

bool FunctionsState::isActivated(byte inFunctionNumber)
{
	return bitRead(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

bool FunctionsState::isActivationChanged(byte inFunctionNumber)
{
	return bitRead(this->activeFlagsSent[inFunctionNumber / 8], inFunctionNumber % 8) != isActivated(inFunctionNumber);
}

void FunctionsState::statesSent() 
{ 
	for (int i = 0; i < 4; i++)
		this->activeFlagsSent[i] = this->activeFlags[i]; 
}

#ifdef DCCPP_DEBUG_MODE
void FunctionsState::printActivated()
{
	for (int i = 0; i < 32; i++)
	{
		if (this->isActivated(i))
		{
			Serial.print(i);
			Serial.print(" ");
		}
	}

	Serial.println("");
}
#endif

// *********************************************************** end of FunctionsState

// *********************************************************** DCCpp class

static bool first = true;

///////////////////////////////////////////////////////////////////////////////
// MAIN ARDUINO LOOP
///////////////////////////////////////////////////////////////////////////////

void DCCpp::loop()
{
#ifdef USE_TEXTCOMMAND
	TextCommand::process();              // check for, and process, and new serial commands
#endif

	if (first)
	{
		first = false;
#if defined(DCCPP_DEBUG_MODE) && defined(DCCPP_PRINT_DCCPP)
		showConfiguration();
#endif
	}

	if (CurrentMonitor::checkTime())
	{      // if sufficient time has elapsed since last update, check current draw on Main and Program Tracks 
		mainMonitor.check();
		progMonitor.check();
	}

#ifdef USE_SENSOR
	Sensor::check();    // check sensors for activated or not
#endif
}

void DCCpp::beginMain(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
	DCCppConfig::DirectionMotorA = inOptionalDirectionMotor;
	DCCppConfig::SignalEnablePinMain = inSignalEnable;	// PWM
	DCCppConfig::CurrentMonitorMain = inCurrentMonitor;

	// If no main line, exit.
	if (DCCppConfig::SignalEnablePinMain == UNDEFINED_PIN)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println("No main track");
#endif
		return;
	}

	mainMonitor.begin(DCCppConfig::CurrentMonitorMain, (char *) "<p2>");

	// CONFIGURE TIMER_1 TO OUTPUT 50% DUTY CYCLE DCC SIGNALS ON OC1B INTERRUPT PINS

	// Direction Pin for Motor Shield Channel A - MAIN OPERATIONS TRACK
	// Controlled by Arduino 16-bit TIMER 1 / OC1B Interrupt Pin
	// Values for 16-bit OCR1A and OCR1B registers calibrated for 1:1 prescale at 16 MHz clock frequency
	// Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle

#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER1 3199
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER1 1599

#define DCC_ONE_BIT_TOTAL_DURATION_TIMER1 1855
#define DCC_ONE_BIT_PULSE_DURATION_TIMER1 927
	if (DCCppConfig::DirectionMotorA != UNDEFINED_PIN)
	{
		pinMode(DCCppConfig::DirectionMotorA, INPUT);      // ensure this pin is not active! Direction will be controlled by DCC SIGNAL instead (below)
		digitalWrite(DCCppConfig::DirectionMotorA, LOW);
	}

	if (inSignalPin != UNDEFINED_PIN)
		pinMode(inSignalPin, OUTPUT); // FOR SHIELDS, THIS ARDUINO OUTPUT PIN MUST BE PHYSICALY CONNECTED TO THE PIN FOR DIRECTION-A OF MOTOR CHANNEL-A

	bitSet(TCCR1A, WGM10);     // set Timer 1 to FAST PWM, with TOP=OCR1A
	bitSet(TCCR1A, WGM11);
	bitSet(TCCR1B, WGM12);
	bitSet(TCCR1B, WGM13);

	bitSet(TCCR1A, COM1B1);    // set Timer 1, OC1B (pin 10/UNO, pin 12/MEGA) to inverting toggle (actual direction is arbitrary)
	bitSet(TCCR1A, COM1B0);

	bitClear(TCCR1B, CS12);    // set Timer 1 prescale=1
	bitClear(TCCR1B, CS11);
	bitSet(TCCR1B, CS10);

	OCR1A = DCC_ONE_BIT_TOTAL_DURATION_TIMER1;
	OCR1B = DCC_ONE_BIT_PULSE_DURATION_TIMER1;

	pinMode(DCCppConfig::SignalEnablePinMain, OUTPUT);   // master enable for motor channel A

	mainRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	bitSet(TIMSK1, OCIE1B);    // enable interrupt vector for Timer 1 Output Compare B Match (OCR1B)    
	digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("beginMain achivied"));
#endif
}

void DCCpp::beginProg(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
	DCCppConfig::DirectionMotorB = inOptionalDirectionMotor;
	DCCppConfig::SignalEnablePinProg = inSignalEnable;
	DCCppConfig::CurrentMonitorProg = inCurrentMonitor;

	// If no programming line, exit.
	if (DCCppConfig::SignalEnablePinProg == UNDEFINED_PIN)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println("No prog track");
#endif
		return;
	}

	progMonitor.begin(DCCppConfig::CurrentMonitorProg, (char *) "<p3>");

	// CONFIGURE EITHER TIMER_0 (UNO) OR TIMER_3 (MEGA) TO OUTPUT 50% DUTY CYCLE DCC SIGNALS ON OC0B (UNO) OR OC3B (MEGA) INTERRUPT PINS

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO

	// Direction Pin for Motor Shield Channel B - PROGRAMMING TRACK
	// Controlled by Arduino 8-bit TIMER 0 / OC0B Interrupt Pin
	// Values for 8-bit OCR0A and OCR0B registers calibrated for 1:64 prescale at 16 MHz clock frequency
	// Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with as-close-as-possible to 50% duty cycle

#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER0 49
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER0 24

#define DCC_ONE_BIT_TOTAL_DURATION_TIMER0 28
#define DCC_ONE_BIT_PULSE_DURATION_TIMER0 14

	if (DCCppConfig::DirectionMotorB != UNDEFINED_PIN)
	{
		pinMode(DCCppConfig::DirectionMotorB, INPUT);      // ensure this pin is not active! Direction will be controlled by DCC SIGNAL instead (below)
		digitalWrite(DCCppConfig::DirectionMotorB, LOW);
	}

	if (inSignalPin != UNDEFINED_PIN)
		pinMode(inSignalPin, OUTPUT);      // THIS ARDUINO OUTPUT PIN MUST BE PHYSICALY CONNECTED TO THE PIN FOR DIRECTION-B OF MOTOR CHANNEL-B

	bitSet(TCCR0A, WGM00);     // set Timer 0 to FAST PWM, with TOP=OCR0A
	bitSet(TCCR0A, WGM01);
	bitSet(TCCR0B, WGM02);

	bitSet(TCCR0A, COM0B1);    // set Timer 0, OC0B (pin 5) to inverting toggle (actual direction is arbitrary)
	bitSet(TCCR0A, COM0B0);

	bitClear(TCCR0B, CS02);    // set Timer 0 prescale=64
	bitSet(TCCR0B, CS01);
	bitSet(TCCR0B, CS00);

	OCR0A = DCC_ONE_BIT_TOTAL_DURATION_TIMER0;
	OCR0B = DCC_ONE_BIT_PULSE_DURATION_TIMER0;

	pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT);   // master enable for motor channel B

	progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	bitSet(TIMSK0, OCIE0B);    // enable interrupt vector for Timer 0 Output Compare B Match (OCR0B)

#else      // Configuration for MEGA

	// Direction Pin for Motor Shield Channel B - PROGRAMMING TRACK
	// Controlled by Arduino 16-bit TIMER 3 / OC3B Interrupt Pin
	// Values for 16-bit OCR3A and OCR3B registers calibrated for 1:1 prescale at 16 MHz clock frequency
	// Resulting waveforms are 200 microseconds for a ZERO bit and 116 microseconds for a ONE bit with exactly 50% duty cycle

#define DCC_ZERO_BIT_TOTAL_DURATION_TIMER3 3199
#define DCC_ZERO_BIT_PULSE_DURATION_TIMER3 1599

#define DCC_ONE_BIT_TOTAL_DURATION_TIMER3 1855
#define DCC_ONE_BIT_PULSE_DURATION_TIMER3 927

	if (DCCppConfig::DirectionMotorB != UNDEFINED_PIN)
	{
		pinMode(DCCppConfig::DirectionMotorB, INPUT);      // ensure this pin is not active! Direction will be controlled by DCC SIGNAL instead (below)
		digitalWrite(DCCppConfig::DirectionMotorB, LOW);
	}

	pinMode(DCC_SIGNAL_PIN_PROG, OUTPUT);      // THIS ARDUINO OUTPUT PIN MUST BE PHYSICALLY CONNECTED TO THE PIN FOR DIRECTION-B OF MOTOR CHANNEL-B

	bitSet(TCCR3A, WGM30);     // set Timer 3 to FAST PWM, with TOP=OCR3A
	bitSet(TCCR3A, WGM31);
	bitSet(TCCR3B, WGM32);
	bitSet(TCCR3B, WGM33);

	bitSet(TCCR3A, COM3B1);    // set Timer 3, OC3B (pin 2) to inverting toggle (actual direction is arbitrary)
	bitSet(TCCR3A, COM3B0);

	bitClear(TCCR3B, CS32);    // set Timer 3 prescale=1
	bitClear(TCCR3B, CS31);
	bitSet(TCCR3B, CS30);

	OCR3A = DCC_ONE_BIT_TOTAL_DURATION_TIMER3;
	OCR3B = DCC_ONE_BIT_PULSE_DURATION_TIMER3;

	pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT);   // master enable for motor channel B

	progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	bitSet(TIMSK3, OCIE3B);    // enable interrupt vector for Timer 3 Output Compare B Match (OCR3B)    

#endif
	digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("beginProg achivied"));
#endif
}

void DCCpp::begin()
{
	programMode = false;
	panicStopped = false;

	DCCppConfig::SignalEnablePinMain = UNDEFINED_PIN;
	DCCppConfig::CurrentMonitorMain = UNDEFINED_PIN;

	DCCppConfig::SignalEnablePinProg = UNDEFINED_PIN;
	DCCppConfig::CurrentMonitorProg = UNDEFINED_PIN;

	DCCppConfig::DirectionMotorA = UNDEFINED_PIN;
	DCCppConfig::DirectionMotorB = UNDEFINED_PIN;

	mainMonitor.begin(UNDEFINED_PIN, "");
	progMonitor.begin(UNDEFINED_PIN, "");

#ifdef SDCARD_CS
	pinMode(SDCARD_CS, OUTPUT);
	digitalWrite(SDCARD_CS, HIGH);     // De-select the SD card
#endif

#ifdef USE_EEPROM
	EEStore::init();                                          // initialize and load Turnout and Sensor definitions stored in EEPROM
	if (EEStore::needsRefreshing())
		EEStore::store();
#endif

#ifdef DCCPP_DEBUG_MODE
	//pinMode(LED_BUILTIN, OUTPUT);
	Serial.println(F("begin achieved"));
#endif

} // begin

#ifdef USE_ETHERNET
void DCCpp::beginEthernet(uint8_t *inMac, uint8_t *inIp, EthernetProtocol inProtocol)
{
	if (inIp != NULL)
		for (int i = 0; i < 4; i++)
			DCCppConfig::EthernetIp[i] = inIp[i];

	for (int i = 0; i < 6; i++)
		DCCppConfig::EthernetMac[i] = inMac[i];

	DCCppConfig::Protocol = inProtocol;

	if (inIp == NULL)
		Ethernet.begin(inMac);                  // Start networking using DHCP to get an IP Address
	else
		Ethernet.begin(inMac, inIp);           // Start networking using STATIC IP Address

	INTERFACE.begin();
#ifdef DCCPP_DEBUG_MODE
	//pinMode(LED_BUILTIN, OUTPUT);
	showConfiguration();
	Serial.println(F("beginEthernet achieved"));
#endif
} // beginEthernet
#endif

  ///////////////////////////////////////////////////////////////////////////////
  // DEFINE THE INTERRUPT LOGIC THAT GENERATES THE DCC SIGNAL
  ///////////////////////////////////////////////////////////////////////////////

  // The code below will be called every time an interrupt is triggered on OCNB, where N can be 0 or 1. 
  // It is designed to read the current bit of the current register packet and
  // updates the OCNA and OCNB counters of Timer-N to values that will either produce
  // a long (200 microsecond) pulse, or a short (116 microsecond) pulse, which respectively represent
  // DCC ZERO and DCC ONE bits.

  // These are hardware-driven interrupts that will be called automatically when triggered regardless of what
  // DCC++ BASE STATION was otherwise processing.  But once inside the interrupt, all other interrupt routines are temporarily disabled.
  // Since a short pulse only lasts for 116 microseconds, and there are TWO separate interrupts
  // (one for Main Track Registers and one for the Program Track Registers), the interrupt code must complete
  // in much less than 58 microseconds, otherwise there would be no time for the rest of the program to run.  Worse, if the logic
  // of the interrupt code ever caused it to run longer than 58 microseconds, an interrupt trigger would be missed, the OCNA and OCNB
  // registers would not be updated, and the net effect would be a DCC signal that keeps sending the same DCC bit repeatedly until the
  // interrupt code completes and can be called again.

  // A significant portion of this entire program is designed to do as much of the heavy processing of creating a properly-formed
  // DCC bit stream upfront, so that the interrupt code below can be as simple and efficient as possible.

  // Note that we need to create two very similar copies of the code --- one for the Main Track OC1B interrupt and one for the
  // Programming Track OCOB interrupt.  But rather than create a generic function that incurs additional overhead, we create a macro
  // that can be invoked with proper parameters for each interrupt.  This slightly increases the size of the code base by duplicating
  // some of the logic for each interrupt, but saves additional time.

  // As structured, the interrupt code below completes at an average of just under 6 microseconds with a worse-case of just under 11 microseconds
  // when a new register is loaded and the logic needs to switch active register packet pointers.

  // THE INTERRUPT CODE MACRO:  R=REGISTER LIST (mainRegs or progRegs), and N=TIMER (0 or 1)

#define DCC_SIGNAL(R,N) \
  if(R.currentBit==R.currentReg->activePacket->nBits){    /* IF no more bits in this DCC Packet */ \
	R.currentBit=0;                                       /*   reset current bit pointer and determine which Register and Packet to process next--- */ \
	if (R.nRepeat>0 && R.currentReg == R.reg) {               /*   IF current Register is first Register AND should be repeated */ \
		R.nRepeat--;                                        /*     decrement repeat count; result is this same Packet will be repeated */ \
	} \
	else if (R.nextReg != NULL) {                           /*   ELSE IF another Register has been updated */ \
		R.currentReg = R.nextReg;                             /*     update currentReg to nextReg */ \
		R.nextReg = NULL;                                     /*     reset nextReg to NULL */ \
		R.tempPacket = R.currentReg->activePacket;            /*     flip active and update Packets */ \
		R.currentReg->activePacket = R.currentReg->updatePacket; \
		R.currentReg->updatePacket = R.tempPacket; \
	} \
	else {                                               /*   ELSE simply move to next Register */ \
		if (R.currentReg == R.maxLoadedReg)                    /*     BUT IF this is last Register loaded */ \
			R.currentReg = R.reg;                               /*       first reset currentReg to base Register, THEN */ \
			R.currentReg++;                                     /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ \
		}                                                     /*   END-ELSE */ \
	}                                                       /* END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ \
	\
	if (R.currentReg->activePacket->buf[R.currentBit / 8] & R.bitMask[R.currentBit % 8]) {     /* IF bit is a ONE */ \
		OCR ## N ## A = DCC_ONE_BIT_TOTAL_DURATION_TIMER ## N;                               /*   set OCRA for timer N to full cycle duration of DCC ONE bit */ \
	OCR ## N ## B=DCC_ONE_BIT_PULSE_DURATION_TIMER ## N;                               /*   set OCRB for timer N to half cycle duration of DCC ONE but */ \
  } else{                                                                              /* ELSE it is a ZERO */ \
	OCR ## N ## A=DCC_ZERO_BIT_TOTAL_DURATION_TIMER ## N;                              /*   set OCRA for timer N to full cycle duration of DCC ZERO bit */ \
	OCR ## N ## B=DCC_ZERO_BIT_PULSE_DURATION_TIMER ## N;                              /*   set OCRB for timer N to half cycle duration of DCC ZERO bit */ \
  }                                                                                    /* END-ELSE */ \
	\
	R.currentBit++;     /* point to next bit in current Packet */

///////////////////////////////////////////////////////////////////////////////
// NOW USE THE ABOVE MACRO TO CREATE THE CODE FOR EACH INTERRUPT

ISR(TIMER1_COMPB_vect) {              // set interrupt service for OCR1B of TIMER-1 which flips direction bit of Motor Shield Channel A controlling Main Track
	DCC_SIGNAL(DCCpp::mainRegs, 1)
}

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO

ISR(TIMER0_COMPB_vect) {              // set interrupt service for OCR1B of TIMER-0 which flips direction bit of Motor Shield Channel B controlling Programming Track
	DCC_SIGNAL(DCCpp::progRegs, 0)
}

#else      // Configuration for MEGA

ISR(TIMER3_COMPB_vect) {              // set interrupt service for OCR3B of TIMER-3 which flips direction bit of Motor Shield Channel B controlling Programming Track
	DCC_SIGNAL(DCCpp::progRegs, 3)
}

#endif

#ifdef DCCPP_PRINT_DCCPP
///////////////////////////////////////////////////////////////////////////////
// PRINT CONFIGURATION INFO TO SERIAL PORT REGARDLESS OF INTERFACE TYPE
// - ACTIVATED ON STARTUP IF SHOW_CONFIG_PIN IS TIED HIGH 

void DCCpp::showConfiguration()
{
	Serial.println(F("*** DCCpp LIBRARY ***"));

	Serial.print(F("VERSION DCC++:      "));
	Serial.println(VERSION);
	Serial.println(F("VERSION DCCpp library: 0.8.0"));
	Serial.print(F("COMPILED:     "));
	Serial.print(__DATE__);
	Serial.print(F(" "));
	Serial.println(__TIME__);

	//Serial.print(F("\nARDUINO:      "));
	//Serial.print(ARDUINO_TYPE);

	//Serial.print(F("\n\nMOTOR SHIELD: "));
	//Serial.print(MOTOR_SHIELD_NAME);

	if (DCCppConfig::SignalEnablePinMain!= UNDEFINED_PIN)
	{
		Serial.print(F("\n\nDCC SIG MAIN(DIR): "));
		Serial.println(DCC_SIGNAL_PIN_MAIN);
		Serial.print(F("   DIRECTION: "));
		Serial.println(DCCppConfig::DirectionMotorA);
		Serial.print(F("   ENABLE(PWM): "));
		Serial.println(DCCppConfig::SignalEnablePinMain);
		Serial.print(F("   CURRENT: "));
		Serial.println(DCCppConfig::CurrentMonitorMain);
	}

	if (DCCppConfig::SignalEnablePinProg!= UNDEFINED_PIN)
	{
		Serial.print(F("\n\nDCC SIG PROG(DIR): "));
		Serial.println(DCC_SIGNAL_PIN_PROG);
		Serial.print(F("   DIRECTION: "));
		Serial.println(DCCppConfig::DirectionMotorB);
		Serial.print(F("   ENABLE(PWM): "));
		Serial.println(DCCppConfig::SignalEnablePinProg);
		Serial.print(F("   CURRENT: "));
		Serial.println(DCCppConfig::CurrentMonitorProg);
	}
#if defined(USE_EEPROM)
#if defined(USE_TURNOUT)
	Serial.print(F("\n\nNUM TURNOUTS: "));
	Serial.println(EEStore::data.nTurnouts);
#endif
#if defined(USE_SENSOR)
	Serial.print(F("     SENSORS: "));
	Serial.println(EEStore::data.nSensors);
#endif
#if defined(USE_OUTPUT)
	Serial.print(F("     OUTPUTS: "));
	Serial.println(EEStore::data.nOutputs);
#endif
#endif

#ifdef USE_TEXTCOMMAND
	Serial.print(F("\n\nINTERFACE:    "));
#ifdef USE_ETHERNET
	Serial.println(F("ETHERNET "));
	Serial.print(F("MAC ADDRESS:  "));
	for (int i = 0; i<5; i++) {
		Serial.print(DCCppConfig::EthernetMac[i], HEX);
		Serial.print(F(":"));
	}
	Serial.println(DCCppConfig::EthernetMac[5], HEX);
//	Serial.print(F("PORT:         "));
//	Serial.println(DCCppConfig::EthernetPort);
	Serial.print(F("IP ADDRESS:   "));
	Serial.println(Ethernet.localIP());

/*#ifdef IP_ADDRESS
	Serial.println(F(" (STATIC)"));
#else
	Serial.println(F(" (DHCP)"));
#endif*/

#else
	Serial.println(F("SERIAL"));
#endif

#endif
//	Serial.print(F("\n\nPROGRAM HALTED - PLEASE RESTART ARDUINO"));

//	while (true);
//		Serial.println("");
}
#endif

void DCCpp::panicStop(bool inStop)
{
	panicStopped = inStop;

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp PanicStop "));
	Serial.println(inStop ? F("pressed"):F("canceled"));
#endif

	/* activate or not the power on rails */

	if (inStop)
		powerOff();
	else
		powerOn();
}

void DCCpp::powerOn()
{
	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinProg, HIGH);
	if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinMain, HIGH);
	INTERFACE.print("<p1>");
#if !defined(USE_ETHERNET)
	INTERFACE.println("");
#endif
}

void DCCpp::powerOff()
{
	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
	if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);
	INTERFACE.print("<p0>");
#if !defined(USE_ETHERNET)
	INTERFACE.println("");
#endif
}

/***************************** Driving functions */

bool DCCpp::setThrottle(volatile RegisterList *inpRegs, int nReg,  int inLocoId, int inStepsNumber, int inNewSpeed, bool inForward)
{
	int val = 0;

	if (panicStopped)
		val = 1;
	else
		if (inNewSpeed > 0)
			val = map(inNewSpeed, 0, inStepsNumber, 2, 127);

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp SetSpeed "));
	Serial.print(inForward?inNewSpeed:-inNewSpeed);
	Serial.print(F("/"));
	Serial.print(inStepsNumber);
	Serial.print(F(" (in Dcc "));
	Serial.print(val);
	Serial.println(F(" )"));
#endif

	inpRegs->setThrottle(nReg, inLocoId, val, inForward);

	return true;
}

void DCCpp::setFunctions(volatile RegisterList *inpRegs, int nReg, int inLocoId, FunctionsState &inStates)
{
#ifdef DCCPP_DEBUG_MODE
	if (inpRegs == &mainRegs)
	{
		if (nReg > MAX_MAIN_REGISTERS)
			Serial.println(F("Invalid register number on main track."));
	}
	else
	{
		if (nReg > MAX_PROG_REGISTERS)
			Serial.println(F("Invalid register number on programming track."));
	}
#endif
	byte flags = 0;

	byte oneByte1 = 128;	// Group one functions F0-F4
	byte twoByte1 = 176;	// Group two F5-F8
	byte threeByte1 = 160;	// Group three F9-F12
	byte fourByte2 = 0;		// Group four F13-F20
	byte fiveByte2 = 0;		// Group five F21-F28

	for (byte func = 0; func <= 28; func++)
	{
		if (func <= 4)
		{
			/*
			*	To set functions F0 - F4 on(= 1) or off(= 0) :
			*
			*    BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
			* BYTE2 : omitted
			*/

			if (inStates.isActivationChanged(func))
				flags |= 1;
			if (inStates.isActivated(func))
			{
				if (func == 0)
					oneByte1 += 16;
				else
					oneByte1 += (1 << (func - 1));
			}
		}
		else if (func <= 8)
		{
			/*
			*	To set functions F5 - F8 on(= 1) or off(= 0) :
			*
			*    BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
			* BYTE2 : omitted
			*/

			if (inStates.isActivationChanged(func))
				flags |= 2;
			if (inStates.isActivated(func))
				twoByte1 += (1 << (func - 5));
		}
		else if (func <= 12)
		{
			/*
			*    To set functions F9 - F12 on(= 1) or off(= 0) :
			*
			*    BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
			* BYTE2 : omitted
			*/

			if (inStates.isActivationChanged(func))
				flags |= 4;
			if (inStates.isActivated(func))
				threeByte1 += (1 << (func - 9));
		}
		else if (func <= 20)
		{
			/*
			*    To set functions F13 - F20 on(= 1) or off(= 0) :
			*
			*    BYTE1 : 222
			* BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
			*/

			if (inStates.isActivationChanged(func))
				flags |= 8;
			if (inStates.isActivated(func))
				fourByte2 += (1 << (func - 13));
		}
		else if (func <= 28)
		{
			/*
			*    To set functions F21 - F28 on(= 1) of off(= 0) :
			*
			*    BYTE1 : 223
			* BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
			*/

			if (inStates.isActivationChanged(func))
				flags |= 16;
			if (inStates.isActivated(func))
				fiveByte2 += (1 << (func - 21));
		}
	}

	if (flags & 1)
		inpRegs->setFunction(nReg, inLocoId, oneByte1, -1);
	if (flags & 2)
		inpRegs->setFunction(nReg, inLocoId, twoByte1, -1);
	if (flags & 4)
		inpRegs->setFunction(nReg, inLocoId, threeByte1, -1);
	if (flags & 8)
		inpRegs->setFunction(nReg, inLocoId, 222, fourByte2);
	if (flags & 16)
		inpRegs->setFunction(nReg, inLocoId, 223, fiveByte2);

	inStates.statesSent();

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp SetFunctions for loco"));
	Serial.print(inLocoId);
	Serial.print(" / Activated : ");
	inStates.printActivated();
#endif
}

void DCCpp::writeCv(volatile RegisterList *inReg, int inLocoId, int inCv, byte inValue)
{
	inReg->writeCVByte(inCv, inValue, 100, 101);

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp WriteCv "));
	Serial.print(inCv);
	Serial.print(F(" : "));
	Serial.println(inValue);
#endif
}

int DCCpp::readCv(volatile RegisterList *inReg, int inLocoId, byte inCv)
{
	return inReg->readCVmain(1, 100+inCv, 100+inCv);
}

void DCCpp::setAccessory(int inAddress, byte inSubAddress, byte inActivate)
{
	mainRegs.setAccessory(inAddress, inSubAddress, inActivate);

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp AccessoryOperation "));
	Serial.print(inAddress);
	Serial.print(F(" / "));
	Serial.print(inSubAddress);
	Serial.print(F(" : "));
	Serial.println(inActivate);
#endif
}


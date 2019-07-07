/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <DCCpp class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

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

void DCCpp::beginMain(uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
	
	DCCppConfig::SignalEnablePinMain = inSignalEnable;
	DCCppConfig::CurrentMonitorMain = inCurrentMonitor;

	// If no main line, exit.
	if (DCCppConfig::SignalEnablePinMain == UNDEFINED_PIN)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println("No main track");
#endif
		return;
	}

	mainMonitor.begin(DCCppConfig::CurrentMonitorMain, DCCppConfig::SignalEnablePinMain, (char *) "<p2>");

	if (inSignalPin != UNDEFINED_PIN) {
		pinMode(inSignalPin, OUTPUT); 
		DCCppConfig::SignalPortMaskMain = digitalPinToBitMask(inSignalPin);
		DCCppConfig::SignalPortInMain = portInputRegister(digitalPinToPort(inSignalPin));
	}

	pinMode(DCCppConfig::SignalEnablePinMain, OUTPUT); 

	/*
	 Use the platform's timer2
	 The timer2 has an 8 bit counter
	 Presclaler = 8 to scale down the 16MHz Arduino processor frequency so that we can use an
	 8bit counter. We need to interrupt every 58us. The frequency of interrupt is thus 
	 1.000.000 / 58 = 17241 = 17,241kHz = f
	 So values are:
	 TCCR2A = 1 << WGM21; Set the TCM mode "Clear Timer on Compare Match"
	 TCCR2B: CS22 = 0; CS21 = 1; CS20 = 0 <- prescaler = 8
	 OCR2A = 115; The frequency of the interrupt, OCR2A = 16MHz / (f * prescaler) - 1
	              so in our case (16.000.000 / (17.241 * 8 )) - 1
	 TIMSK2 = 1 << OCIE2A; Enable timer compare interrupt
	*/

	noInterrupts();

	TCCR2A = 1 << WGM21; // CTC
    TCCR2B = (0 << CS22) | (1 << CS21) | (0 << CS20); // prescale 8

	OCR2A = 115;              // Gives interrupt every 58us (for 16 Mhz)
	bitSet  (TIMSK2, OCIE2A); // enable interrupt

	interrupts();


	mainRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("beginMain achivied"));
#endif
}

void DCCpp::beginProg(uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
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

	progMonitor.begin(DCCppConfig::CurrentMonitorProg, DCCppConfig::SignalEnablePinProg, (char *) "<p3>");

	if (inSignalPin != UNDEFINED_PIN) {
		pinMode(inSignalPin, OUTPUT); 
		DCCppConfig::SignalPortMaskProg = digitalPinToBitMask(inSignalPin);
		DCCppConfig::SignalPortInProg = portInputRegister(digitalPinToPort(inSignalPin));
	}


	pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT); 

	// same code as in beginMain, it uses the same timer2.
	noInterrupts();

	bitSet  (TCCR2A, WGM21);  // CTC
	TCCR2A = 1 << WGM21; // CTC
    TCCR2B = (0 << CS22) | (1 << CS21) | (0 << CS20); // prescale 8
	OCR2A = 115;              // Gives interrupt every 58us (for 16 Mhz)
	bitSet  (TIMSK2, OCIE2A); // enable interrupt

	interrupts();

	progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

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

	DCCppConfig::SignalPortMaskMain = 0;
	DCCppConfig::SignalPortMaskProg = 0;

	mainMonitor.begin(UNDEFINED_PIN, UNDEFINED_PIN, "");
	progMonitor.begin(UNDEFINED_PIN, UNDEFINED_PIN, "");

#ifdef SDCARD_CS
	pinMode(SDCARD_CS, OUTPUT);
	digitalWrite(SDCARD_CS, HIGH);     // De-select the SD card
#endif

#ifdef USE_EEPROM
	EEStore::init();     // initialize and load Turnout and Sensor definitions stored in EEPROM
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

	DCCPP_INTERFACE.begin();
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

  // The code below will be called every time an timer interrupt is triggered. 
  // It is designed to read the current bit of the current register packet and
  // toggles the specified pin for a long (200 microsecond) or a short (116 microsecond) pulse,
  // which respectively represent DCC ZERO and DCC ONE bits.

  // Note that we need to create two very similar copies of the code --- one for the Main Track and one for the
  // Programming Track.  But rather than create a generic function that incurs additional overhead, we create a macro
  // that can be invoked with proper parameters for each interrupt.  This slightly increases the size of the code base by duplicating
  // some of the logic for each interrupt, but saves additional time.

  // THE INTERRUPT CODE MACRO:  R=REGISTER LIST (mainRegs or progRegs), and N=Main or Prog postfix

#define DCC_NEXT_BIT(R) \
	if(R.currentBit==R.currentReg->activePacket->nBits){  /* IF no more bits in this DCC Packet */ \
	R.currentBit = 0;                                     /*   reset current bit pointer and determine which Register and Packet to process next--- */ \
	if (R.nRepeat>0 && R.currentReg == R.reg){               /*   IF current Register is first Register AND should be repeated */ \
		R.nRepeat--;                                      /*     decrement repeat count; result is this same Packet will be repeated */ \
	}  \
	else if (R.nextReg != NULL){                           /*   ELSE IF another Register has been updated */ \
		R.currentReg = R.nextReg;                         /*     update currentReg to nextReg */ \
		R.nextReg = NULL;                                 /*     reset nextReg to NULL */ \
		R.tempPacket = R.currentReg->activePacket;        /*     flip active and update Packets */ \
		R.currentReg->activePacket = R.currentReg->updatePacket; \
		R.currentReg->updatePacket = R.tempPacket; \
	} \
	else {                                                /*   ELSE simply move to next Register */ \
		if (R.currentReg == R.maxLoadedReg)                  /*     BUT IF this is last Register loaded */ \
			R.currentReg = R.reg;                         /*       first reset currentReg to base Register, THEN */ \
		R.currentReg++;                                   /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ \
		}                                                 /*   END-ELSE */ \
	}                                                     /* END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ \
	if(R.currentReg->activePacket->buf[R.currentBit/8] & R.bitMask[R.currentBit%8]) {  \
		/* For 1 bit, we need 1 periods of 58us timer ticks for each signal level */ \
		R.timerPeriods = 1; \
		R.timerPeriodsLeft = 2; \
	} else {  /* ELSE it is a ZERO bit */ \
		/* For 0 bit, we need 2 period of 58us timer ticks for each signal level. */ \
		R.timerPeriods = 2; \
		R.timerPeriodsLeft = 4; \
	}         /* END-ELSE */ \
	                         \
	R.currentBit++;     /* point to next bit in current Packet */  
  
///////////////////////////////////////////////////////////////////////////////

/* 
   For each bit, toggle pin twice: when timer counts to timerPeriods of
   the register, and when timer counts to 0. Then next bit is activated.
 */ 

#define CHECK_TIMER_PERIOD(R,N)                    \
	R.timerPeriodsLeft--;                          \
	if(R.timerPeriodsLeft == R.timerPeriods) {     \
		*DCCppConfig::SignalPortIn ## N = DCCppConfig::SignalPortMask ## N; \
	}                                              \
	if(R.timerPeriodsLeft == 0) {                  \
		*DCCppConfig::SignalPortIn ## N = DCCppConfig::SignalPortMask ## N; \
		DCC_NEXT_BIT(R);                           \
	}                                              \

// NOW USE THE ABOVE MACRO TO CREATE THE CODE FOR EACH INTERRUPT

ISR(TIMER2_COMPA_vect) { 
	if(DCCppConfig::SignalPortMaskMain != 0)
  		CHECK_TIMER_PERIOD(DCCpp::mainRegs, Main)
  	if(DCCppConfig::SignalPortMaskProg != 0)
  		CHECK_TIMER_PERIOD(DCCpp::progRegs, Prog)
	//*DCCppConfig::SignalPortInMain = DCCppConfig::SignalPortMaskMain; 
}


#ifdef DCCPP_PRINT_DCCPP
///////////////////////////////////////////////////////////////////////////////
// PRINT CONFIGURATION INFO TO SERIAL PORT REGARDLESS OF INTERFACE TYPE
// - ACTIVATED ON STARTUP IF SHOW_CONFIG_PIN IS TIED HIGH 

void DCCpp::showConfiguration()
{
	Serial.println(F("*** DCCpp LIBRARY ***"));

	Serial.print(F("VERSION DCC++:      "));
	Serial.println(VERSION);
	Serial.println(F(DCCPP_LIBRARY_VERSION));
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

void DCCpp::powerOn(bool inMain, bool inProg)
{
	if (inProg && DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinProg, HIGH);

	if (inMain && DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinMain, HIGH);
	DCCPP_INTERFACE.print("<p1>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
#endif
}

void DCCpp::powerOff(bool inMain, bool inProg)
{
	if (inProg && DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
	if (inMain && DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);
	DCCPP_INTERFACE.print("<p0>");
#if !defined(USE_ETHERNET)
	DCCPP_INTERFACE.println("");
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

int DCCpp::identifyLocoId(volatile RegisterList *inReg)
{
	int  id = -1;
	int temp;
	temp = inReg->readCV(29, 100, 200);
	if ((temp != -1) && (bitRead(temp, 5))) {
		// long address : get CV#17 and CV#18
		id = inReg->readCV(18, 100, 200);
		if (id != -1) {
			temp = inReg->readCV(17, 100, 200);
			if (temp != -1) {
				id = id + ((temp - 192) << 8);
			}
		}
	}
	else {
		// short address: read only CV#1
		id = inReg->readCV(1, 100, 200);
	}
	return(id);
}

void DCCpp::writeCv(volatile RegisterList *inReg, int inCv, byte inValue, int callBack, int callBackSub)
{
	inReg->writeCVByte(inCv, inValue, callBack, callBackSub);

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("DCCpp WriteCv "));
	Serial.print(inCv);
	Serial.print(F(" : "));
	Serial.println(inValue);
#endif
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


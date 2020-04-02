/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <DCCpp signal for ESP32 modules>
*************************************************************/

#include "Arduino.h"

#if defined(ARDUINO_ARCH_ESP32)

#include "DCCpp.h"

// Define constants for DCC Signal pattern

// this controls the timer tick frequency
#define DCC_TIMER_PRESCALE 80

// number of microseconds for sending a zero via the DCC encoding
#define DCC_ZERO_BIT_TOTAL_DURATION 196
// number of microseconds for each half of the DCC signal for a zero
#define DCC_ZERO_BIT_PULSE_DURATION 98

// number of microseconds for sending a one via the DCC encoding
#define DCC_ONE_BIT_TOTAL_DURATION 116
// number of microseconds for each half of the DCC signal for a one
#define DCC_ONE_BIT_PULSE_DURATION 58

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

  // THE INTERRUPT CODE MACRO:  R=REGISTER LIST pointer (&mainRegs or &progRegs)

#define DCC_SIGNAL(R) \
	byte nextBitToSendLocal = 0; \
  if(R->currentBit == R->currentReg->activePacket->nBits){    /* IF no more bits in this DCC Packet */ \
	R->currentBit = 0;                                       /*   reset current bit pointer and determine which Register and Packet to process next--- */ \
	if (R->nRepeat > 0 && R->currentReg == R->reg) {               /*   IF current Register is first Register AND should be repeated */ \
		R->nRepeat--;                                        /*     decrement repeat count; result is this same Packet will be repeated */ \
	} \
	else if (R->nextReg != NULL) {                           /*   ELSE IF another Register has been updated */ \
		R->currentReg = R->nextReg;                             /*     update currentReg to nextReg */ \
		R->nextReg = NULL;                                     /*     reset nextReg to NULL */ \
		R->tempPacket = R->currentReg->activePacket;            /*     flip active and update Packets */ \
		R->currentReg->activePacket = R->currentReg->updatePacket; \
		R->currentReg->updatePacket = R->tempPacket; \
	} \
	else {                                               /*   ELSE simply move to next Register */ \
		if (R->currentReg == R->maxLoadedReg)                    /*     BUT IF this is last Register loaded */ \
			R->currentReg = R->reg;                               /*       first reset currentReg to base Register, THEN */ \
			R->currentReg++;                                     /*     increment current Register (note this logic causes Register[0] to be skipped when simply cycling through all Registers) */ \
		}                                                     /*   END-ELSE */ \
	}                                                       /* END-IF: currentReg, activePacket, and currentBit should now be properly set to point to next DCC bit */ \
	\
	if (R->currentReg->activePacket->buf[R->currentBit / 8] & R->bitMask[R->currentBit % 8]) {     /* IF bit is a ONE */ \
		nextBitToSendLocal = 1;					 \
  } else{                                                                              /* ELSE it is a ZERO */ \
		nextBitToSendLocal = 0;					 \
	}                                                                                    /* END-ELSE */ \
	\
	R->currentBit++;     /* point to next bit in current Packet */

///////////////////////////////////////////////////////////////////////////////
// NOW USE THE ABOVE MACRO TO CREATE THE CODE FOR EACH INTERRUPT

class SignalGenerator
{
public:
	SignalGenerator(byte inTimerIndex) { this->timerIndex = inTimerIndex; this->_pulseTimer = NULL; }

	void configureSignal(uint8_t inSignalPin);
	void startSignal(bool inDebugMode);
	void stopSignal(void);

	uint8_t _signalPin;
	byte timerIndex;
	volatile RegisterList *pRegisters;

	hw_timer_t* _pulseTimer;

	bool halfPulse;
};

SignalGenerator *pMainSignal = NULL, *pProgSignal = NULL;
#ifdef DCCPP_DEBUG_MODE
long debugDelay = 0;
int curr = LOW;
#endif
bool pulseMain = false;
bool pulseProg = false;

void IRAM_ATTR signalGeneratorPulseTimerMain(void)
{
#ifdef DCCPP_DEBUG_MODE
	/*	if (millis() - debugDelay > 1000)
		{
			debugDelay = millis();
			curr = curr == LOW ? HIGH : LOW;
			digitalWrite(2, curr);
		}*/
#endif

	pulseMain = !pulseMain;
	if (pulseMain)
	{
		digitalWrite(pMainSignal->_signalPin, LOW);
		return;
	}
	DCC_SIGNAL(pMainSignal->pRegisters)
	if (nextBitToSendLocal)
		timerAlarmWrite(pMainSignal->_pulseTimer, DCC_ONE_BIT_PULSE_DURATION, true);
	else
		timerAlarmWrite(pMainSignal->_pulseTimer, DCC_ZERO_BIT_PULSE_DURATION, true);

	timerWrite(pMainSignal->_pulseTimer, 0);
	timerAlarmEnable(pMainSignal->_pulseTimer);
	digitalWrite(pMainSignal->_signalPin, HIGH);
}

void IRAM_ATTR signalGeneratorPulseTimerProg(void)
{
#ifdef DCCPP_DEBUG_MODE
	/*	if (millis() - debugDelay > 1000)
		{
			debugDelay = millis();
			curr = curr == LOW ? HIGH : LOW;
			digitalWrite(2, curr);
		}*/
#endif

	pulseProg = !pulseProg;
	if (pulseProg)
	{
		digitalWrite(pProgSignal->_signalPin, LOW);
		return;
	}

	DCC_SIGNAL(pProgSignal->pRegisters)
	if (nextBitToSendLocal)
		timerAlarmWrite(pProgSignal->_pulseTimer, DCC_ONE_BIT_PULSE_DURATION, true);
	else
		timerAlarmWrite(pProgSignal->_pulseTimer, DCC_ZERO_BIT_PULSE_DURATION, true);
	timerWrite(pProgSignal->_pulseTimer, 0);
	timerAlarmEnable(pProgSignal->_pulseTimer);
	digitalWrite(pProgSignal->_signalPin, HIGH);
}

void SignalGenerator::startSignal(bool inDebugMode)
{
	//log_i("[%s] Configuring Timer(%d) for generating DCC Signal (Half Wave)", _name.c_str(), 2 * timerIndex + 1);
	this->_pulseTimer = timerBegin(this->timerIndex, inDebugMode ? DCC_TIMER_PRESCALE/10 :  DCC_TIMER_PRESCALE, true);
	//log_i("[%s] Attaching interrupt handler to Timer(%d)", _name.c_str(), 2 * timerIndex + 1);
	timerAttachInterrupt(this->_pulseTimer, this == pMainSignal ? &signalGeneratorPulseTimerMain : &signalGeneratorPulseTimerProg, true);
	//log_i("[%s] Configuring alarm on Timer(%d) to %dus", _name.c_str(), 2 * timerIndex + 1, DCC_ONE_BIT_TOTAL_DURATION / 2);
	timerAlarmWrite(this->_pulseTimer, DCC_ONE_BIT_PULSE_DURATION, true);
	//log_i("[%s] Setting load on Timer(%d) to zero", _name.c_str(), 2 * timerIndex + 1);
	timerWrite(this->_pulseTimer, 0);

	//log_i("[%s] Enabling alarm on Timer(%d)", _name.c_str(), 2 * timerIndex + 1);
	timerAlarmEnable(this->_pulseTimer);

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Signal started for ");
	Serial.print(this == pMainSignal ? "Main track" : "Prog track");
	Serial.print(" on pin ");
	Serial.println(this->_signalPin);
#endif
}

void SignalGenerator::stopSignal(void) 
{
	//log_i("[%s] Shutting down Timer(%d) (Half Wave)", _name.c_str(), 2 * timerIndex + 1);
	timerStop(this->_pulseTimer);
	timerAlarmDisable(this->_pulseTimer);
	timerDetachInterrupt(this->_pulseTimer);
	timerEnd(this->_pulseTimer);

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Signal stoppedfor ");
	Serial.print(this == pMainSignal ? "Main track" : "Prog track");
	Serial.print(" on pin ");
	Serial.println(this->_signalPin);
#endif
	// give enough time for any timer ISR calls to complete before draining
	// the packet queue and returning
	delay(250);
}

void SignalGenerator::configureSignal(uint8_t inSignalPin)
{
	this->_signalPin = inSignalPin;

	// force the directionPin to low since it will be controlled by the DCC timer
	pinMode(inSignalPin, INPUT);
	digitalWrite(inSignalPin, LOW);
	pinMode(inSignalPin, OUTPUT);
	this->startSignal(false);
}

void DCCpp::beginMainDccSignal(uint8_t inSignalPin)
{
	DCCpp::mainRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	pMainSignal = new SignalGenerator(0);
	pMainSignal->pRegisters = &DCCpp::mainRegs;
	pMainSignal->configureSignal(inSignalPin);

	if (DCCppConfig::DirectionMotorA != UNDEFINED_PIN)
	{
		pinMode(DCCppConfig::DirectionMotorA, INPUT);      // ensure this pin is not active! Direction will be controlled by DCC SIGNAL instead (below)
		digitalWrite(DCCppConfig::DirectionMotorA, LOW);
	}

	if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		pinMode(DCCppConfig::SignalEnablePinMain, OUTPUT);

#ifdef DCCPP_DEBUG_MODE
	Serial.println("Main track DCC ESP32 started.");
#endif
}

void DCCpp::beginProgDccSignal(uint8_t inSignalPin)
{
	DCCpp::progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	pProgSignal = new SignalGenerator(1);
	pProgSignal->pRegisters = &DCCpp::progRegs;
	pProgSignal->configureSignal(inSignalPin);

	if (DCCppConfig::DirectionMotorB != UNDEFINED_PIN)
	{
		pinMode(DCCppConfig::DirectionMotorB, INPUT);      // ensure this pin is not active! Direction will be controlled by DCC SIGNAL instead (below)
		digitalWrite(DCCppConfig::DirectionMotorB, LOW);
	}

	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT);

#ifdef DCCPP_DEBUG_MODE
	Serial.println("Prog track DCC ESP32 started.");
#endif
}

void DCCpp::setDebugDccMode()
{
	if (pMainSignal != NULL)
	{
		pMainSignal->stopSignal();
		pMainSignal->startSignal(true);
	}

	if (pProgSignal != NULL)
	{
		pProgSignal->stopSignal();
		pProgSignal->startSignal(true);
	}
}

#endif
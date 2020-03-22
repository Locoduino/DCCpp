/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <DCCpp signal for Uno/Nano ARduino>
*************************************************************/

#include "Arduino.h"

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)      // Configuration for UNO/MEGA

#include "DCCpp.h"

void DCCpp::beginMainDccSignal(uint8_t inSignalPin)
{
#ifndef USE_ONLY1_INTERRUPT
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

	if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
		pinMode(DCCppConfig::SignalEnablePinMain, OUTPUT);   // master enable for motor channel A

	mainRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	bitSet(TIMSK1, OCIE1B);    // enable interrupt vector for Timer 1 Output Compare B Match (OCR1B)    
#else
	if (inSignalPin != UNDEFINED_PIN) {
		pinMode(inSignalPin, OUTPUT); 
		DCCppConfig::SignalPortMaskMain = digitalPinToBitMask(inSignalPin);
		DCCppConfig::SignalPortInMain = portInputRegister(digitalPinToPort(inSignalPin));
	}

	if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
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
#endif
}

void DCCpp::beginProgDccSignal(uint8_t inSignalPin)
{
#ifndef USE_ONLY1_INTERRUPT
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

	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
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

	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT);   // master enable for motor channel B

	progRegs.loadPacket(1, RegisterList::idlePacket, 2, 0);    // load idle packet into register 1    

	bitSet(TIMSK3, OCIE3B);    // enable interrupt vector for Timer 3 Output Compare B Match (OCR3B)    

#endif
	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
		
#else
	if (inSignalPin != UNDEFINED_PIN) {
		pinMode(inSignalPin, OUTPUT); 
		DCCppConfig::SignalPortMaskProg = digitalPinToBitMask(inSignalPin);
		DCCppConfig::SignalPortInProg = portInputRegister(digitalPinToPort(inSignalPin));
	}

	if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
		pinMode(DCCppConfig::SignalEnablePinProg, OUTPUT);

	// same code as in beginMain, it uses the same timer2.
	noInterrupts();

	bitSet  (TCCR2A, WGM21);  // CTC
	TCCR2A = 1 << WGM21; // CTC
    TCCR2B = (0 << CS22) | (1 << CS21) | (0 << CS20); // prescale 8
	OCR2A = 115;              // Gives interrupt every 58us (for 16 Mhz)
	bitSet  (TIMSK2, OCIE2A); // enable interrupt

	interrupts();
#endif
}

#ifndef USE_ONLY1_INTERRUPT
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

#else
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
#endif

void DCCpp::setDebugDccMode()
{
	bitClear(TCCR1B, CS12);    // set Timer 1 prescale=8 - SLOWS NORMAL SPEED BY FACTOR OF 8
	bitSet(TCCR1B, CS11);
	bitClear(TCCR1B, CS10);

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_NANO)      // Configuration for UNO

	bitSet(TCCR0B, CS02);    // set Timer 0 prescale=256 - SLOWS NORMAL SPEED BY A FACTOR OF 4
	bitClear(TCCR0B, CS01);
	bitClear(TCCR0B, CS00);

#else                     // Configuration for MEGA

	bitClear(TCCR3B, CS32);    // set Timer 3 prescale=8 - SLOWS NORMAL SPEED BY A FACTOR OF 8
	bitSet(TCCR3B, CS31);
	bitClear(TCCR3B, CS30);

#endif

	CLKPR = 0x80;           // THIS SLOWS DOWN SYSYEM CLOCK BY FACTOR OF 256
	CLKPR = 0x08;           // BOARD MUST BE RESET TO RESUME NORMAL OPERATIONS
}

#endif
//-------------------------------------------------------------------
#ifndef __DCCpp_hpp__
#define __DCCpp_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

class FunctionsState
{
	private:
		// Four bytes : 32 bits : from function 0 to 28, maximum for Dcc.
		// A bit at true is an activated function.
		byte activeFlags[4];

		inline byte byteNumber(byte inFunctionNumber) { return inFunctionNumber / 8; }
		inline byte bitNumber(byte inFunctionNumber) { return inFunctionNumber % 8; }

	public:
		FunctionsState();

		void clear();
		void activate(byte inFunctionNumber);
		void inactivate(byte inFunctionNumber);
		bool isActivated(byte inFunctionNumber);

#ifdef DCCPP_DEBUG_MODE
		void printActivated();
#endif
};

class DCCppClass
{
	private:
		bool programMode;
		bool panicStopped;

		bool setThrottle(volatile RegisterList *inReg, int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft);
		int readCv(volatile RegisterList *inReg, int inLocoId, byte inCvId);
		void writeCv(volatile RegisterList *inReg, int inLocoId, int inCvId, byte inCvValue);
		void setFunctions(volatile RegisterList *inReg, int nReg, int inLocoId, FunctionsState inStates);

	public:
		static volatile RegisterList mainRegs, progRegs;
		static CurrentMonitor mainMonitor;
		static CurrentMonitor progMonitor;
	
		DCCppClass();
		
	public:
		// begins
		void begin();
		void beginMain(uint8_t OptionalDirectionMotor, uint8_t Dummy, uint8_t SignalEnablePin, uint8_t CurrentMonitor);
		void beginProg(uint8_t OptionalDirectionMotor, uint8_t Dummy, uint8_t SignalEnablePin, uint8_t CurrentMonitor);
#ifdef USE_ETHERNET
		void beginEthernet(uint8_t *inMac, uint8_t *inIp);
#endif

		// DCCpp global functions
		void loop();
		void panicStop(bool inStop);
		void powerOn();
		void powerOff();
		inline float getCurrentMain() { return this->mainMonitor.pin == 255 ? 0 : mainMonitor.current; }
		inline float getCurrentProg() { return this->progMonitor.pin == 255 ? 0 : progMonitor.current; }

		// Main driving functions
		inline bool setSpeedMain(int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft) { return this->setThrottle(&(this->mainRegs), nReg, inLocoId, inStepsNumber, inNewSpeed, inToLeft); }
		inline int readCvMain(int inLocoId, byte inCvId) { return this->readCv(&(this->mainRegs), inLocoId, inCvId); }
		inline void writeCvMain(int inLocoId, int inCvId, byte inValue) { this->writeCv(&(this->mainRegs), inLocoId, inCvId, inValue); }
		inline void setFunctionsMain(int nReg, int inLocoId, FunctionsState inStates) { this->setFunctions(&(this->mainRegs), nReg, inLocoId, inStates); }

		// Prog driving functions
		inline bool setSpeedProg(int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft) { return this->setThrottle(&(this->progRegs), nReg, inLocoId, inStepsNumber, inNewSpeed, inToLeft); }
		inline int readCvProg(int inLocoId, byte inCvId) { return this->readCv(&(this->progRegs), inLocoId, inCvId); }
		inline void writeCvProg(int inLocoId, int inCvId, byte inValue) { this->writeCv(&(this->progRegs), inLocoId, inCvId, inValue); }
		inline void setFunctionsProg(int nReg, int inLocoId, FunctionsState inStates) { this->setFunctions(&(this->progRegs), nReg, inLocoId, inStates); }

		// Accessories
		void setAccessory(int inAddress, byte inSubAddress, byte inActivate);

#ifdef DCCPP_PRINT_DCCPP
		static void showConfiguration();
#endif

		static DCCppClass DCCppInstance;
};

#define DCCpp	DCCppClass::DCCppInstance

//-------------------------------------------------------------------
#endif
//-------------------------------------------------------------------

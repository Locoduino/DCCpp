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

		inline byte ByteNumber(byte inFunctionNumber) { return inFunctionNumber / 8; }
		inline byte BitNumber(byte inFunctionNumber) { return inFunctionNumber % 8; }

	public:
		FunctionsState();

		void Clear();
		void Activate(byte inFunctionNumber);
		void Inactivate(byte inFunctionNumber);
		bool IsActivated(byte inFunctionNumber);

#ifdef DCCPP_DEBUG_MODE
		void printActivated();
#endif
};

class DCCppClass
{
	private:
		bool programMode;
		bool panicStopped;

		bool SetThrottle(volatile RegisterList *inReg, int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft);
		int ReadCv(volatile RegisterList *inReg, int inLocoId, byte inCvId);
		void WriteCv(volatile RegisterList *inReg, int inLocoId, int inCvId, byte inCvValue);
		void SetFunctions(volatile RegisterList *inReg, int nReg, int inLocoId, FunctionsState inStates);

	public:
		static volatile RegisterList mainRegs, progRegs;
		static CurrentMonitor MainMonitor;
		static CurrentMonitor ProgMonitor;
	
		DCCppClass();
		
	public:
		// begins
		void begin();
		void beginMain(uint8_t OptionalDirectionMotor, uint8_t Dummy, uint8_t SignalEnablePin, uint8_t CurrentMonitor);
		void beginProg(uint8_t OptionalDirectionMotor, uint8_t Dummy, uint8_t SignalEnablePin, uint8_t CurrentMonitor);
#ifndef USE_ETHERNET
		void beginEthernet(uint8_t *inMac, IPAddress inIpAddress, int inPort);
#endif

		// DCCpp global functions
		void loop();
		void PanicStop(bool inStop);
		void StartProgramMode();
		void EndProgramMode();

		// Main driving functions
		inline bool SetSpeedMain(int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft) { return this->SetThrottle(&(this->mainRegs), nReg, inLocoId, inStepsNumber, inNewSpeed, inToLeft); }
		inline int ReadCvMain(int inLocoId, byte inCvId) { return this->ReadCv(&(this->mainRegs), inLocoId, inCvId); }
		inline void WriteCvMain(int inLocoId, int inCvId, byte inValue) { this->WriteCv(&(this->mainRegs), inLocoId, inCvId, inValue); }
		inline void SetFunctionsMain(int nReg, int inLocoId, FunctionsState inStates) { this->SetFunctions(&(this->mainRegs), nReg, inLocoId, inStates); }

		// Prog driving functions
		inline bool SetSpeedProg(int nReg, int inLocoId, int inStepsNumber, int inNewSpeed, bool inToLeft) { return this->SetThrottle(&(this->progRegs), nReg, inLocoId, inStepsNumber, inNewSpeed, inToLeft); }
		inline int ReadCvProg(int inLocoId, byte inCvId) { return this->ReadCv(&(this->progRegs), inLocoId, inCvId); }
		inline void WriteCvProg(int inLocoId, int inCvId, byte inValue) { this->WriteCv(&(this->progRegs), inLocoId, inCvId, inValue); }
		inline void SetFunctionsProg(int nReg, int inLocoId, FunctionsState inStates) { this->SetFunctions(&(this->progRegs), nReg, inLocoId, inStates); }

#ifdef DCCPP_PRINT_DCCPP
		static void showConfiguration();
#endif

		static DCCppClass DCCppInstance;
};

#define DCCpp	DCCppClass::DCCppInstance

//-------------------------------------------------------------------
#endif
//-------------------------------------------------------------------

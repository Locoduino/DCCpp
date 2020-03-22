// ArduinoIno.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include "Arduino.h"

//#include "../examples/Autotest/Autotest.ino"
//#include "../examples/EthernetDcc/EthernetDcc.ino"
//#include "../examples/EthernetDcc/WiFiDcc.ino"
//#include "../examples/MaxiDcc/MaxiDcc.ino"
//#include "../examples/MiniDcc/MiniDcc.ino"
#include "../examples/SerialDcc/SerialDcc.ino"
//#include "../../..//TPC/DCC/DCC.ino"

//#include "TestExe.ino"

int main()
{
	emulatorSetup();
	setup();

	while (1)
	{
		emulatorLoop();
		loop();
	}
}

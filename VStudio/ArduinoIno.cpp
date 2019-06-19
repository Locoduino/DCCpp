// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
//#include <iostream>

#include "Arduino.h"
#include "DIO2.h"
//#include "ArduiEmulator.hpp" 
#include "NamedPipes.h" 

//#include "../examples/Autotest/Autotest.ino"
//#include "../examples/EthernetDcc/EthernetDcc.ino"
//#include "../examples/MaxiDcc/MaxiDcc.ino"
//#include "../examples/MiniDcc/MiniDcc.ino"
//#include "../examples/SerialDcc/SerialDcc.ino"
#include "../../..//TPC/DCC/DCC.ino"

//#include "TestExe.ino"

int main()
{
	NamedPipesBegin(L"\\\\.\\pipe\\ToArduino", L"\\\\.\\pipe\\FromArduino");
	
	NamedPipesSend(MessagesTypes::Reset, _T(""));

	pinSetup();

	setup();

	while (1)
	{
		NamedPipesIdle();
		loop();
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

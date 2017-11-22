#include "Arduino.h"

#include "..\..\libraries\DCCpp\examples\MiniDcc\MiniDcc.ino"
//#include "..\..\libraries\DCCpp\examples\MaxiDcc\MaxiDcc.ino"
//#include "..\..\libraries\DCCpp\examples\SerialDcc\SerialDcc.ino"
//#include "..\..\libraries\DCCpp\examples\EthernetDcc\EthernetDcc.ino"

//#include "TestExe.ino"

using namespace System;
using namespace System::Windows::Forms;

void OnMouseDown(System::Object ^sender, System::Windows::Forms::MouseEventArgs ^e);
void OnClick(System::Object ^sender, System::EventArgs ^e);
[STAThread]
void Main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	ArduiEmulator::ArduinoForm form(setup, loop);
	form.buttonStart->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(&OnMouseDown);
		
	ArduiEmulator::Arduino::_pinSetup();
	Application::Run(%form);
}

void OnMouseDown(System::Object ^sender, System::Windows::Forms::MouseEventArgs ^e)
{
	ArduiEmulator::Arduino::_pinSetup();
}

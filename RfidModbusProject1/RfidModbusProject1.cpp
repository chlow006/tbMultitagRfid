#include <iostream>
#include "RfidTben.h"
#include <Windows.h>

extern "C" {
#include <modbus.h>
}

int display_options();

int main()
{
	RfidTben mock2;
	int ch = 0;

	std::cout<<"This is sample test program for TBEN_S2_2RFID_4DXP\n";
	std::cout <<"Connecting via modbus to TBEN_S2_2RFID_4DXP\n";
	if (mock2.connectModbus("192.168.1.13") != 0) {
		std::cout << "Connection failed. Please check hardware and resolve error. Exiting program\n";
	}
	else {
		std::cout << "Connected.\n";
		
		do
		{
			std::cout << "What's the channel connected to rfid reader?\n";
			if (!(std::cin >> ch)) {//error occurred
				std::cout << "invalid input" << std::endl;
				std::cin.clear();//Clear the error
				std::cin.ignore(); //discard input
			}
		} while (ch != 0 && ch != 1);
		RfidTben::ModbusAddress chX_commandCode;
		RfidTben::ModbusAddress chX_startAddr;
		RfidTben::ModbusAddress chX_length;
		RfidTben::ModbusAddress chX_tagCounter;
		RfidTben::ModbusAddress chX_byteAvailable;
		RfidTben::ModbusAddress chX_inputTag;

		if (ch == 0) {
			chX_commandCode = RfidTben::ch0_commandCode;
			chX_startAddr = RfidTben::ch0_startAddr;
			chX_length = RfidTben::ch0_length;
			chX_tagCounter = RfidTben::ch0_tagCounter;
			chX_byteAvailable = RfidTben::ch0_byteAvailable;
			chX_inputTag = RfidTben::ch0_inputTag;
		}
		else {
			chX_commandCode = RfidTben::ch1_commandCode;
			chX_startAddr = RfidTben::ch1_startAddr;
			chX_length = RfidTben::ch1_length;
			chX_tagCounter = RfidTben::ch1_tagCounter;
			chX_byteAvailable = RfidTben::ch1_byteAvailable;
			chX_inputTag = RfidTben::ch1_inputTag;
		}

		while (true) {
			int option = display_options();
			int loopCount = 0;
			char input2 = '0';
			uint16_t * intptr = mock2.awRFID_input;
			switch (option) {
				case 1:
					mock2.Rfid_changeMode(RfidTben::Idle, chX_commandCode);
					break;
				case 2:
					mock2.Rfid_changeStartAddr((uint16_t)1, chX_startAddr); // set start address to enable grouping of tags
					std::this_thread::sleep_for(50ms);
					mock2.Rfid_changeByteLength((uint16_t)16, chX_length);
					std::this_thread::sleep_for(50ms);
					mock2.Rfid_changeMode(RfidTben::StartContinousMode, chX_commandCode);
					std::cout << "Enter 'c' to stop scanning. Enter something else to update...\n";
					while (input2 != 'c') {
						std::this_thread::sleep_for(500ms);
						mock2.Rfid_readTagCounter(chX_tagCounter);
						std::cout << "Number of tag detected: " << mock2.wTagCounter << "\n";
						std::cin >> input2;
					}
					mock2.Rfid_changeMode(RfidTben::StopContinousMode, chX_commandCode);
					break;
				case 3:
					mock2.Rfid_readTag(ch);
					mock2.Rfid_parseTagDetected(ch);
					std::cout << "\n";
					break;
				case 4:
					printf("awRFID_input: %d  %d %d %d %d %d %d %d %d %d\n",
						intptr[0], intptr[1], intptr[2], intptr[3],
						intptr[4], intptr[5], intptr[6], intptr[7],
						intptr[8], intptr[9]);
					break;
				case 7:
					mock2.Rfid_scanTag(ch, 3000);
					mock2.Rfid_parseTagDetected(ch);
					break;
				case 8:
					mock2.Rfid_changeMode(RfidTben::Reset, chX_commandCode);
					break;
				case 9:
					std::cout << "ENDing PROGRAM.....\n";
					return 0;
				default:
					break;
			}
		}
	}
}

int display_options() {
	int input=1;
	std::cout << "Please choose your options\n";
	std::cout << "1: idle\n";
	std::cout << "2: scan with continuous mode\n";
	std::cout << "3: read and parse tags\n";
	std::cout << "4: print awRFID_input\n";
	std::cout << "7: scan for 3 seconds and parse\n";
	std::cout << "8: reset\n";
	std::cout << "9: END PROGRAM\n";
	std::cout << "Please choose your options....>   ";
	do
	{
		if (!(std::cin >> input)) {//error occurred
			std::cout << "invalid input" << std::endl;
			std::cin.clear();//Clear the error
			std::cin.ignore(); //discard input
		}
	} while (input < 1 || input>9);

	return (int)input;
}
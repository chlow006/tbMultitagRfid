#include <iostream>
#include "RfidTben.h"
#include <Windows.h>

extern "C" {
#include <modbus.h>
}

int display_options();

int main()
{
	/*RfidTben mock1;
	mock1.connectModbus("192.168.1.13");
	
	int rc = mock1.Rfid_scanTag(0,3000);
	/*for (int i = 0; i < rc; i++) {
		printf("read %d registers reg[%d]=%d (0x%X)\n", rc, i, mock1.awRFID_input[i], mock1.awRFID_input[i]);
	}*/
	/*mock1.Rfid_parseTagDetected(0);
	printf("read %d tags\n", mock1.wTagCounter);*/
	
	RfidTben mock2;

	std::cout<<"This is sample test program for TBEN_S2_2RFID_4DXP\n";
	std::cout <<"Connecting via modbus to TBEN_S2_2RFID_4DXP\n";
	if (mock2.connectModbus("192.168.1.13") != 0) {
		std::cout << "Connection failed. Please check hardware and resolve error. Exiting program\n";
	}
	else {
		std::cout << "Connected.\n";
		int ch = 0;
		std::cout << "What's the channel connected to rfid reader?\n";
		std::cin >> ch;
		
		RfidTben::ModbusAddress chAddr;
		if (ch = 0) {
			chAddr = RfidTben::ch0_commandCode;
		}
		else {
			chAddr = RfidTben::ch1_commandCode;
		}

		while (true) {
			int option = display_options();
			int loopCount = 0;
			char input2 = '0';
			switch (option) {
				case 1:
					mock2.Rfid_changeMode(RfidTben::Idle, RfidTben::ch0_commandCode);
					break;
				case 2:
					mock2.Rfid_changeStartAddr((uint16_t)1, RfidTben::ch0_startAddr); // set start address to enable grouping of tags
					std::this_thread::sleep_for(50ms);
					mock2.Rfid_changeByteLength((uint16_t)16, RfidTben::ch0_length);
					std::this_thread::sleep_for(50ms);
					mock2.Rfid_changeMode(RfidTben::StartContinousMode, RfidTben::ch0_commandCode);
					std::cout << "Enter 'c' to stop scanning. Enter something else to update...\n";
					while (input2 != 'c') {
						std::this_thread::sleep_for(500ms);
						mock2.Rfid_readTagCounter(RfidTben::ch0_tagCounter);
						std::cout << "Number of tag detected: " << mock2.wTagCounter << "\n";
						std::cin >> input2;
					}
					mock2.Rfid_changeMode(RfidTben::StopContinousMode, RfidTben::ch0_commandCode);
					break;
				case 3:
					mock2.Rfid_readByteAvailable(RfidTben::ch0_byteAvailable);
					std::this_thread::sleep_for(50ms);
					loopCount = 0;
					if (mock2.wByteAvailable==0) {
						continue;
					}
					while (mock2.wByteAvailable > 128) {
						mock2.Rfid_changeByteLength(128, RfidTben::ch0_length);
						mock2.Rfid_changeMode(mock2.GetData, RfidTben::ch0_commandCode);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_readTagInput(64, RfidTben::ch0_inputTag, loopCount);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_changeMode(RfidTben::Idle, RfidTben::ch0_commandCode);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_readByteAvailable(RfidTben::ch0_byteAvailable); //update byte available and still unread
						loopCount++;
					}
					mock2.Rfid_changeByteLength(mock2.wByteAvailable, RfidTben::ch0_length);
					mock2.Rfid_changeMode(RfidTben::GetData, RfidTben::ch0_commandCode);
					std::this_thread::sleep_for(100ms);
					mock2.Rfid_readTagInput(mock2.wByteAvailable / 2, RfidTben::ch0_inputTag, loopCount);
					mock2.Rfid_parseTagDetected(0);
					std::cout << "\n";
					break;
				case 7:
					mock2.Rfid_scanTag(0, 3000);
					mock2.Rfid_parseTagDetected(0);
					break;
				case 8:
					mock2.Rfid_changeMode(RfidTben::Reset, RfidTben::ch0_commandCode);
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
	std::cout << "7: scan for 3 seconds and parse\n";
	std::cout << "8: reset\n";
	std::cout << "9: END PROGRAM\n";
	std::cout << "Please choose your options....>   ";
	std::cin >> input;
	if (input < 1 || input>9)
		input = display_options();
	return input;
}
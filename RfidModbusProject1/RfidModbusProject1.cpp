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
		
		RfidTben::ModbusAddress chX_commandCode;
		RfidTben::ModbusAddress chX_startAddr;
		RfidTben::ModbusAddress chX_length;
		RfidTben::ModbusAddress chX_tagCounter;
		RfidTben::ModbusAddress chX_byteAvailable;
		RfidTben::ModbusAddress chX_inputTag;

		if (ch = '0') {
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
		std::cout << chX_commandCode << "\n";
		std::cout << chX_length << "\n";

		while (true) {
			int option = display_options();
			int loopCount = 0;
			char input2 = '0';
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
					mock2.Rfid_readByteAvailable(chX_byteAvailable);
					std::this_thread::sleep_for(50ms);
					loopCount = 0;
					if (mock2.wByteAvailable==0) {
						continue;
					}
					while (mock2.wByteAvailable > 128) {
						mock2.Rfid_changeByteLength(128, chX_length);
						mock2.Rfid_changeMode(mock2.GetData, chX_commandCode);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_readTagInput(64, chX_inputTag, loopCount);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_changeMode(RfidTben::Idle, chX_commandCode);
						std::this_thread::sleep_for(200ms);
						mock2.Rfid_readByteAvailable(chX_byteAvailable); //update byte available and still unread
						loopCount++;
					}
					mock2.Rfid_changeByteLength(mock2.wByteAvailable, chX_length);
					mock2.Rfid_changeMode(RfidTben::GetData, chX_commandCode);
					std::this_thread::sleep_for(100ms);
					mock2.Rfid_readTagInput(mock2.wByteAvailable / 2, chX_inputTag, loopCount);
					mock2.Rfid_parseTagDetected(0);
					std::cout << "\n";
					break;
				case 7:
					mock2.Rfid_scanTag(0, 3000);
					mock2.Rfid_parseTagDetected(0);
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
	std::cout << "7: scan for 3 seconds and parse\n";
	std::cout << "8: reset\n";
	std::cout << "9: END PROGRAM\n";
	std::cout << "Please choose your options....>   ";
	std::cin >> input;
	if (input < 1 || input>9) {
		input = 1;
		std::cout << "Please choose an appropriate number   ";
	}
	return (int)input;
}
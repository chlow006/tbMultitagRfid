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
			uint32_t input3;
			int input4;
			uint32_t input5;
			uint8_t epcChange[16] = { 0x4c,0x51,0x31,0x34,0x78,0x31,0x34,0x4f,
								0x4e,0x4c,0x30,0x30,0x30,0x30,0x30,0x39};
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
					mock2.Rfid_readEPC(ch);
					mock2.Rfid_parseEPCDetected(ch);
					std::cout << "\n";
					break;
				case 4:
					mock2.Rfid_changeEPCLength(&mock2.awRFID_input[1], &epcChange[0], 6, 8, 0);
					break;
				case 5:
					mock2.Rfid_initAllTags(ch, EXPECTED_EPC_LENGTH, 2000);
					break;
				case 6:
					std::cout << "Enter index of the tag you want to read (0....maxNumberOfTags) ...\n";
					std::cin >> input4;
					input5 = mock2.Rfid_readData(ch, input4, EXPECTED_EPC_LENGTH);
					printf("Data:%ld\n", input5);
					break;
				case 7:
					std::cout << "Enter a number to add to memory...\n";
					std::cin >> input3;
					mock2.Rfid_WriteData(ch, 0, EXPECTED_EPC_LENGTH, input3);
					printf("Data written\n");
					break;
				case 8:
					mock2.Rfid_incrementAllTags(ch, EXPECTED_EPC_LENGTH,2000);
					break;
				case 9:
					mock2.Rfid_changeMode(RfidTben::Reset, chX_commandCode);
					break;
				case 0:
					std::cout << "ENDing PROGRAM.....\n";
					return 0;
				default:
					break;
			}
		}
	}
}

int display_options() {
	int input = 1;
	std::cout << "Please choose your options\n";
	std::cout << "1: idle\n";
	std::cout << "2: scan with continuous mode\n";
	std::cout << "3: read and parse tags\n";
	std::cout << "4: change preset EPC\n";
	std::cout << "5: Rfid_initAllTags\n";
	std::cout << "6: read Userdata\n";
	std::cout << "7: write Userdata\n";
	std::cout << "8: Increment all detected tags\n";
	std::cout << "9: reset\n";
	std::cout << "0: END PROGRAM\n";
	std::cout << "Please choose your options....>   ";
	do
	{
		if (!(std::cin >> input)) {//error occurred
			std::cout << "invalid input" << std::endl;
			std::cin.clear();//Clear the error
			std::cin.ignore(); //discard input
		}
	} while (input < 0 || input>9);

	return (int)input;
}
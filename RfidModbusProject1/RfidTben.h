//#pragma once
#ifndef TRFID_H
#define TRFID_H

//including <Windows.h> leads to compile error. Please use Sleep() outside of class - 26nov2021

#include <chrono>
#include <thread>
#include <stdio.h>
#include <cstdint>
#include <string>


extern "C" {
#include <modbus.h>
}

using namespace std::chrono_literals;
using std::string;

class RfidTben
{
	modbus_t * modbusHandler;

	uint16_t awModb_Write[7]; // 0-1:commandcode,	2-3:start address, 4-5: length , 6: CmdTimeOut//
	uint16_t wCommandCode;
	uint16_t wdStartAddress;
	uint16_t wLEN;
	uint16_t wCmdTimeOut;


	uint16_t wTagCounter;
	uint16_t wByteAvailable;
	uint8_t abRFID_input[128];
	string asRFID[10];

public:

	uint16_t awRFID_input[64];
	enum RfidCommand {
		Idle = 0x0000,
		Inventory = 0x0001,
		Read = 0x0002,
		Write = 0x0004,
		StartContinousMode = 0x0010,
		StopContinousMode = 0x0012,
		GetData = 0x0011,
		Reset = 0x8000
	};

	int  connectModbus(modbus_t * mb, const char * ipAddress);
	int  connectModbus(const char * ipAddress);
	int  readModbusInput(uint16_t startAddr, uint16_t numByteToRead);
	int  writeModbus(uint16_t startAddr, int valueToWrite);

	int Rfid_changeMode(RfidCommand command);
	int Rfid_changeStartAddr(uint16_t addr);
	int Rfid_changeStartAddr(uint32_t addr);
	int Rfid_changeByteLength(uint16_t len);
	int Rfid_changeByteLength(uint32_t len);


	int Rfid_readTagInput(uint16_t len);
	int Rfid_readTagCounter();
	uint16_t Rfid_readByteAvailable();

	int Rfid_scanTag();  //detects number of tags, stores the epc information into the in
	string wordToAscii(uint16_t wordSrc);
	int Rfid_parseTagDetected();

	~RfidTben();

};

#endif

/*******************************************************************
RFID Modbus code Version 1
Ver1: Initial commit by Low Chang Hong (29 Nov 2021)
******************************************************************/
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


	uint16_t wByteAvailable;
	uint8_t abRFID_input[128];
	string asRFID[10];

public:

	uint16_t wTagCounter;
	uint16_t awRFID_input[64];
	typedef enum  {
		Idle = 0x0000,
		Inventory = 0x0001,
		Read = 0x0002,
		Write = 0x0004,
		StartContinousMode = 0x0010,
		StopContinousMode = 0x0012,
		GetData = 0x0011,
		Reset = 0x8000
	}RfidCommand;
	
	#define TBEN_S2_2RFID_4DXP
	#ifdef TBEN_S2_2RFID_4DXP
	typedef enum {
		ch0_commandCode = 0x800,
		ch0_memoryArea = 0x801,
		ch0_startAddr = 0x802,
		ch0_length = 0x804,
		ch0_EPClength = 0x805,

		ch0_tagCounter = 0x05,
		ch0_byteAvailable = 0x06,
		ch0_readFrag = 0x07,
		ch0_writeFrag = 0x07,
		ch0_tagPresent = 0xb,
		ch0_inputTag = 0x0c,

		ch1_commandCode = 0x84c,
		ch1_memoryArea = 0x84d,
		ch1_startAddr = 0x84e,
		ch1_length = 0x850,
		ch1_EPClength = 0x851,

		ch1_tagCounter = 0x51,
		ch1_byteAvailable = 0x52,
		ch1_readFrag = 0x53,
		ch1_writeFrag = 0x53,
		ch1_tagPresent = 0x57,
		ch1_inputTag = 0x58,
	}ModbusAddress;
	#endif

	int  connectModbus(modbus_t * mb, const char * ipAddress);
	int  connectModbus(const char * ipAddress);
	int  writeModbus(uint16_t startAddr, int valueToWrite);

	int Rfid_changeMode(RfidCommand command, ModbusAddress MBaddr);
	int Rfid_changeStartAddr(uint16_t addr, ModbusAddress MBaddr);
	int Rfid_changeStartAddr(uint32_t addr, ModbusAddress MBaddr);
	int Rfid_changeByteLength(uint16_t len, ModbusAddress MBaddr);

	int Rfid_readTagInput(uint16_t len, ModbusAddress MBaddr);
	int Rfid_readTagCounter(ModbusAddress MBaddr);
	uint16_t Rfid_readByteAvailable(ModbusAddress MBaddr);

	int Rfid_scanTag(int channel,int timeout);  //detects number of tags, stores the epc information into the in
	string wordToAscii(uint16_t wordSrc);
	int Rfid_parseTagDetected(int channel);

	void disconnectModbus();
	~RfidTben();

};

#endif

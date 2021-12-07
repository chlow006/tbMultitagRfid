/*******************************************************************
RFID Modbus code Version 1
Ver1: Initial commit by Low Chang Hong (29 Nov 2021)

Note for usage, unless explicitly mentioned, all length are in bytes, not word
******************************************************************/
#ifndef TRFID_H
#define TRFID_H

//including <Windows.h> leads to compile error. Please use Sleep() outside of class - 26nov2021
#include <chrono>
#include <thread>
#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <string>

extern "C" {
#include <modbus.h>
}

using namespace std::chrono_literals;
using std::string;

#define EXPECTED_BYTE_LENGTH (EXPECTED_EPC_LENGTH + 4)
#define EXPECTED_EPC_LENGTH 16

class RfidTben
{
	modbus_t * modbusHandler;

	uint16_t awModb_Write[7]; // 0-1:commandcode,	2-3:start address, 4-5: length , 6: CmdTimeOut//
	uint16_t wCommandCode;
	uint16_t wdStartAddress;
	uint16_t wLEN;
	uint16_t wCmdTimeOut;
	
public:

	uint16_t awEPC[50][EXPECTED_EPC_LENGTH];
	uint16_t wByteAvailable;
	string asRFIDname[50];
	uint16_t wTagCounter;
	uint16_t awRFID_input[300];
	uint16_t awRFID_output[30];
	typedef enum  {
		Idle = 0x0000,
		Inventory = 0x0001,
		Read = 0x0002,
		Write = 0x0004,
		StartContinousMode = 0x0010,
		StopContinousMode = 0x0012,
		GetData = 0x0011,
		ChangeEPCLength = 0x0007,
		Reset = 0x8000
	}RfidCommand;
	
	#define TBEN_S2_2RFID_4DXP
	#ifdef TBEN_S2_2RFID_4DXP
	
	typedef enum {
		Killpassword = 0,
		EPC = 1,
		TID = 2,
		userArea = 3,
		accessPassword = 4,
		epcSize = 5
	}RfidMemoryArea;

	typedef enum {
		ch0_commandCode = 0x800,
		ch0_memoryArea = 0x801, //MSB 8bit, LSB 8bit is loopcounter
		ch0_startAddr = 0x802,
		ch0_length = 0x804,
		ch0_EPClength = 0x805,
		ch0_outputData = 0x80c,

		ch0_tagCounter = 0x05,
		ch0_byteAvailable = 0x06,
		ch0_readFrag = 0x07,
		ch0_writeFrag = 0x07,
		ch0_tagPresent = 0xb,
		ch0_inputTag = 0x0c,

		ch1_commandCode = 0x84c,
		ch1_memoryArea = 0x84d, //MSB 8bit, LSB 8bit is loopcounter
		ch1_startAddr = 0x84e,
		ch1_length = 0x850,
		ch1_EPClength = 0x851,
		ch1_outputData = 0x0858,

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
	int Rfid_changeMemoryArea(RfidMemoryArea area, ModbusAddress MBaddr);

	int Rfid_changeStartAddr(uint16_t addr, ModbusAddress MBaddr);
	int Rfid_changeStartAddr(uint32_t addr, ModbusAddress MBaddr);
	int Rfid_changeByteLength(uint16_t len, ModbusAddress MBaddr);
	int Rfid_changeEpcLength(uint16_t epclen, ModbusAddress MBaddr);

	int Rfid_changeOutputData(int wordLen, ModbusAddress MBaddr);

	int Rfid_changeEPCLength(uint16_t * oldepc, uint8_t * newepc, int old_wordLen, uint16_t new_wordLen, int channel);
	int Rfid_readTagInput(uint16_t wordLen, ModbusAddress MBaddr, int iteration);
	int Rfid_readTagCounter(ModbusAddress MBaddr);
	uint16_t Rfid_readByteAvailable(ModbusAddress MBaddr);
	
	
	uint32_t Rfid_readData(int channel, int rfidIndex, int epcbyteLen);
	int Rfid_WriteData(int channel, int rfidIndex, int epcbyteLen, uint32_t writeByteData);
	int Rfid_scanEPC(int channel, int timeout);  //detects number of tags, stores the epc information
	int Rfid_readEPC(int channel);  //reads buffer stores the epc information
	
	string convertExtendedAscii(uint8_t hex);
	string wordToAscii(uint16_t wordSrc);
	string wordToByteString(uint16_t wordSrc);
	int Rfid_parseEPCDetected(int channel);

	void disconnectModbus();
	~RfidTben();

	//test function with hardcoding
	int Rfid_changeEPCLength(int testmode);
};

#endif

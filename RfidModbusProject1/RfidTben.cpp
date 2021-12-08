#include "RfidTben.h"

int RfidTben::connectModbus(const char * ipAddress) {

	modbusHandler = modbus_new_tcp(ipAddress, 502);
	if (modbusHandler == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	if (modbus_connect(modbusHandler) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(modbusHandler);
		return -1;
	}
	return 0;
}

int RfidTben::connectModbus(modbus_t * mb, const char * ipAddress) {

	mb = modbus_new_tcp(ipAddress, 502);
	if (mb == NULL) {
		fprintf(stderr, "Unable to allocate libmodbus context\n");
		return -1;
	}
	if (modbus_connect(mb) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(mb);
		return -1;
	}
	return 0;	
}

int  RfidTben::writeModbus(uint16_t startAddr, int valueToWrite) {
	int wc = 0;
	wc = modbus_write_register(modbusHandler, startAddr, valueToWrite);
	if (wc == -1) {
		fprintf(stderr, "write failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	
	return wc;
}

int RfidTben::Rfid_changeMode(RfidCommand command, ModbusAddress MBaddr) {
	if (command == Reset) {
		memset(awRFID_input, 0, sizeof(awRFID_input));
	}
	return writeModbus(MBaddr, command);
}

int RfidTben::Rfid_changeStartAddr(uint16_t addr, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, addr);
}

int RfidTben::Rfid_changeMemoryArea(RfidMemoryArea area, ModbusAddress MBaddr) {

	return writeModbus(MBaddr, (area<<8));
}	

int RfidTben::Rfid_changeStartAddr(uint32_t addr, ModbusAddress MBaddr) { //to be tested
	int wc;
	uint16_t temp[2];
	memcpy(temp, &addr, 2);
	wc = writeModbus(MBaddr, temp[0]);
	wc = writeModbus((MBaddr+1), temp[1]);
	return wc;
}

int RfidTben::Rfid_changeByteLength(uint16_t len, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, len);
}

int RfidTben::Rfid_changeEpcLength(uint16_t epclen, ModbusAddress MBaddr) {
	return writeModbus(MBaddr, epclen);
}

int RfidTben::Rfid_changeOutputData(int wordLen, ModbusAddress MBaddr) {
	//lenth of output data to write in word
	int wc = modbus_write_registers(modbusHandler, MBaddr, wordLen, awRFID_output);
	if (wc == -1) {
		fprintf(stderr, "write failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wc;
}

int RfidTben::Rfid_changeEPCLength(uint16_t * oldepc, uint8_t * newepc, int old_wordLen, uint16_t new_wordLen, int channel) {
//step 1: Set Idle starting state andstarting address
//step 2: Set new length of new epc
//step 3: change 'Length of UID/EPC' to original EPC length of targetted RFID tag
//step 4: Create necessary output write buffer (EPC of the tag to be written followed by New EPC with new length)
//step 5: Send command
	switch (channel) {
	case 0:
		Rfid_changeMode(Idle, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMemoryArea(EPC, ch0_memoryArea);
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch0_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)(new_wordLen) * 2, ch0_length); 
		std::this_thread::sleep_for(50ms);
		//copy bytes of old epc
		memcpy(awRFID_output, oldepc, old_wordLen * 2);
		//change 'Length of UID/EPC' 
		Rfid_changeEpcLength(old_wordLen * 2, ch0_EPClength);
		//copy new epc
		memcpy(awRFID_output + (old_wordLen), newepc, (new_wordLen) * 2); //copy in bytes
		Rfid_changeOutputData((old_wordLen + new_wordLen), ch0_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(ChangeEPCLength, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		return 0;
	case 1:
		Rfid_changeMode(Idle, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMemoryArea(EPC, ch1_memoryArea);
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch1_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)(new_wordLen) * 2, ch1_length);
		std::this_thread::sleep_for(50ms);
		//copy bytes of old epc
		memcpy(awRFID_output, oldepc, old_wordLen * 2);
		//change 'Length of UID/EPC'
		Rfid_changeEpcLength(old_wordLen * 2, ch1_EPClength);
		//copy new epc
		memcpy(awRFID_output + (old_wordLen), newepc, (new_wordLen) * 2); //copy in bytes
		Rfid_changeOutputData((old_wordLen + new_wordLen), ch1_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(ChangeEPCLength, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		return 0;
	default:
		return -1;
	}
}


int RfidTben::Rfid_readTagInput(uint16_t wordLen, ModbusAddress MBaddr, int iteration) {
	int rc = 0;
	int arrayOffset = (128/2)*iteration; //offset by max number of WORD per read
	rc = modbus_read_registers(modbusHandler, MBaddr, wordLen, awRFID_input+arrayOffset);
	if (rc == -1) {
		fprintf(stderr, "read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return rc;
}

int RfidTben::Rfid_readTagCounter(ModbusAddress MBaddr) {
	int tc = modbus_read_registers(modbusHandler, MBaddr, 1, &wTagCounter);
	if (tc == -1) {
		fprintf(stderr, "tag count read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wTagCounter;
}

uint16_t RfidTben::Rfid_readResponseCode(ModbusAddress MBaddr) {
	uint16_t responseCode;
	int resc = modbus_read_registers(modbusHandler, MBaddr, 1, &responseCode);
	if (resc == -1) {
		fprintf(stderr, "responseCode read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	xEC_Error = (responseCode & 0x4000) >> 14;
	xEC_Busy = (responseCode & 0x8000) >> 15;
	return responseCode;
}

uint16_t RfidTben::Rfid_readStatusCode(ModbusAddress MBaddr) {
	uint16_t statusCode;
	int resc = modbus_read_registers(modbusHandler, MBaddr, 1, &statusCode);
	if (resc == -1) {
		fprintf(stderr, "responseCode read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	xTagPresent = (statusCode & 0x1);
	xAntennaDetune = (statusCode & 0x10) >> 4;
	xParNoSupported = (statusCode & 0x20) >> 5;
	xError = (statusCode & 0x40) >> 6;
	xNotConnected = (statusCode & 0x80) >> 7;
	xRWHeadSwitchON = (statusCode & 0x100) >> 8;
	xContinuousModeActive = (statusCode & 0x200) >> 9;
	return statusCode;
}

uint16_t RfidTben::Rfid_readErrorCode(ModbusAddress MBaddr) {
	int ec = modbus_read_registers(modbusHandler, MBaddr, 1, &wErrorCode);
	if (ec == -1) {
		fprintf(stderr, "error code read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wErrorCode;
}

uint16_t RfidTben::Rfid_readByteAvailable(ModbusAddress MBaddr) {
	int tc = modbus_read_registers(modbusHandler, MBaddr, 1, &wByteAvailable);
	if (tc == -1) {
		fprintf(stderr, "byte available read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wByteAvailable;
}


int RfidTben::Rfid_scanEPC(int channel, int scanTime) {
	//Flow chart: Continuous mode without interruption before reading data (TBEN-S2-2RFID-4DXP Manual page 270)
	//step 1: Set group address to detect unique IDs(Leaving it at zero will cause hardware to keep polling old tags)
	//step 2: Set length 
	//step 3: Start polling in continuous mode
	//step 4: Stop polling in continuous mode
	//step 5: Retrieve data

	int loopCount = 0;

	switch (channel)
	{
	case 0:
		Rfid_changeMode(Idle, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMemoryArea(EPC, ch0_memoryArea);
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)1, ch0_startAddr); //enable grouping of EPC
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)16, ch0_length); //set to positive value
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(StartContinousMode, ch0_commandCode);
		std::this_thread::sleep_for(std::chrono::milliseconds(scanTime));
		Rfid_changeMode(StopContinousMode, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_readEPC(0);
		return 0;
		break;
	case 1:
		Rfid_changeMode(Idle, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMemoryArea(EPC, ch1_memoryArea);
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)1, ch1_startAddr);//enable grouping of EPC
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)16, ch1_length);//set to positive value
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(StartContinousMode, ch1_commandCode);
		std::this_thread::sleep_for(std::chrono::milliseconds(scanTime));
		Rfid_changeMode(StopContinousMode, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_readEPC(1);
		return 0;
		break;
	default:
		printf("non-existent channel selected. Please choose again\n");
		return 0;

	}
}

int RfidTben::Rfid_readEPC(int channel) {
	int loopCount = 0;
	int byteRead = 0;
	int rc = 0;
	switch (channel)
	{
	case 0:
		Rfid_readByteAvailable(ch0_byteAvailable);
		std::this_thread::sleep_for(50ms);
		Rfid_changeEpcLength(0, ch0_EPClength); //read all tags
		while (wByteAvailable > 128) {
			printf("While: loopCount:%d  wByteAvailable: %d\n", loopCount,wByteAvailable);
			Rfid_changeByteLength(128, ch0_length);
			Rfid_changeMode(GetData, ch0_commandCode);
			std::this_thread::sleep_for(100ms);
			Rfid_readTagInput(64, ch0_inputTag, loopCount);
			std::this_thread::sleep_for(100ms);
			Rfid_changeMode(Idle, ch0_commandCode);
			std::this_thread::sleep_for(100ms);
			Rfid_readByteAvailable(ch0_byteAvailable); //update byte available and still unread
			loopCount++;
			byteRead += 128;
		}
		rc = Rfid_changeByteLength(wByteAvailable, ch0_length);
		printf("rc=%d loopCount:%d  wByteAvailable: %d\n", rc, loopCount, wByteAvailable);
		std::this_thread::sleep_for(100ms);
		Rfid_changeMode(GetData, ch0_commandCode);
		std::this_thread::sleep_for(100ms);
		return (byteRead + Rfid_readTagInput(wByteAvailable / 2, ch0_inputTag, loopCount));
		break;
	case 1:
		Rfid_readByteAvailable(ch1_byteAvailable);
		std::this_thread::sleep_for(50ms);
		while (wByteAvailable > 128) {
			Rfid_changeByteLength(128, ch1_length);
			Rfid_changeMode(GetData, ch1_commandCode);
			std::this_thread::sleep_for(100ms);
			Rfid_readTagInput(64, ch1_inputTag, loopCount);
			std::this_thread::sleep_for(100ms);
			Rfid_changeMode(Idle, ch1_commandCode);
			std::this_thread::sleep_for(100ms);
			Rfid_readByteAvailable(ch1_byteAvailable); //update byte available and still unread
			loopCount++;
			byteRead += 128;
		}
		Rfid_changeByteLength(wByteAvailable, ch1_length);
		Rfid_changeMode(GetData, ch1_commandCode);
		std::this_thread::sleep_for(100ms);
		return (byteRead + Rfid_readTagInput(wByteAvailable / 2, ch1_inputTag, loopCount));
		break;
	default:
		printf("non-existent channel selected. Please choose again\n");
		return 0;

	}


}

string RfidTben::convertExtendedAscii(uint8_t hexInput) {
	string exStr = "";
	int temp[2];
	temp[0] = hexInput & 0xf;
	temp[1] = hexInput >> 4;

	if (temp[1] < 10) 
		exStr.push_back(('0' + temp[1]));
	else 
		exStr.push_back(('0' + temp[1] + 7));

	if (temp[0] < 10) 
		exStr.push_back(('0' + temp[0]));
	else 
		exStr.push_back(('0' + temp[0] + 7));

	return exStr;
}

string RfidTben::wordToAscii(uint16_t wordSrc) {
	uint8_t hex[2];
	string str = "";
	hex[0] = wordSrc & 0xff;
	hex[1] = (wordSrc >> 8) & 0xff;
	if (hex[0] < 127) 
		str.push_back((char)hex[0]);
	else 
		str += convertExtendedAscii(hex[0]);

	if (hex[1] < 127) 
		str.push_back((char)hex[1]);
	else 
		str += convertExtendedAscii(hex[1]);
	
	return str;
}

//parse hex to string
string RfidTben::wordToByteString(uint16_t wordSrc) {
	uint8_t hex;
	char temp[4];
	char swap[5] = { '0','0','0','0','\0' };
	for (int i = 0; i < 4; i++) {
		hex = (wordSrc >> (i * 4)) & 0xf;
		if (hex < 10) {
			temp[i] = ('0' + hex);
		}
		else {
			temp[i] = ('0' + hex + 7);
		}
	}
	swap[0] = temp[1];
	swap[1] = temp[0];
	swap[2] = temp[3];
	swap[3] = temp[2];
	string str(swap);
	return str;
}

int RfidTben::Rfid_parseEPCDetected(int channel) {
	//assume rfid is a 12byte hex number.\
	//every rfid will readout will consist of the 16byte in total
	//byte 1: Data size
	//byte 2: Block type (1 UID/EPC/RSSI etc.  2: Read data)
	//byte 3-14: EPC number
	//byte 15-16: Number of the read operations (LSB -> MSB) 
	switch (channel) {
		case 0:
			Rfid_readTagCounter(ch0_tagCounter);
			break;
		case 1:
			Rfid_readTagCounter(ch1_tagCounter);
			break;
	}
	string str;
	for (int i = 0; i < wTagCounter; i++) {
		str = "";
		for (int j = 0; j < (EXPECTED_EPC_LENGTH/2); j++) {
			int index = (EXPECTED_BYTE_LENGTH/2)*i + 1 + j ;
			str+=wordToAscii(awRFID_input[index]);
			awEPC[i][j] = awRFID_input[index];
			
		}
		asRFIDname[i] = str;
		std::cout << "i: " << i << " string: " << asRFIDname[i] << std::endl;
	}
	return 0;
}


uint32_t RfidTben::Rfid_readData(int channel, int rfidIndex, int epcbyteLen) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set memory area to change
	//step 2: Define EPC of target tag in "process output data"
	//step 3: change 'Length of UID/EPC' to EPC length of targetted RFID tag
	//step 4: Create 'Length' to length of data to read and starting address of 'input buffer'
	//step 5: Send command
	//step 6: Read 'inpput process buffer' for response
	uint32_t result;
	Rfid_changeMode(Idle, ch0_commandCode); 
	std::this_thread::sleep_for(20ms);
	switch (channel) {
	case 0: 
		Rfid_changeMemoryArea(userArea, ch0_memoryArea);
		std::this_thread::sleep_for(50ms);
		memcpy(awRFID_output, awEPC[rfidIndex], epcbyteLen); //copy EPC of target tag
		Rfid_changeEpcLength(epcbyteLen, ch0_EPClength); //length of target epc
		std::this_thread::sleep_for(50ms);
		Rfid_changeOutputData(epcbyteLen/2, ch0_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)4, ch0_length);//read doubleword
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch0_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(Read, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_readTagInput(2, ch0_inputTag, 0);
		std::this_thread::sleep_for(50ms);
		memcpy(&result, awRFID_input, 4);
		return result;
	case 1: 
		Rfid_changeMemoryArea(userArea, ch1_memoryArea);
		std::this_thread::sleep_for(50ms);
		memcpy(awRFID_output, awEPC[rfidIndex], epcbyteLen); //copy EPC of target tag
		Rfid_changeEpcLength(epcbyteLen, ch1_EPClength); //length of target epc
		std::this_thread::sleep_for(50ms);
		Rfid_changeOutputData(epcbyteLen / 2, ch1_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)4, ch1_length);//read doubleword
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch1_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(Read, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		Rfid_readTagInput(2, ch1_inputTag, 0);
		std::this_thread::sleep_for(50ms);
		memcpy(&result, awRFID_input, 4);
		return result;
		break;
	default:
		return -1;
	}
}

int RfidTben::Rfid_WriteData(int channel, int rfidIndex, int epcbyteLen, uint32_t writeByteData) {
	//step 0: Set Idle intermediate state (Changing to a different state is required for successive read)
	//step 1: Set memory area to change
	//step 2: Define EPC of target tag, followed by data to be written, into "process output data"
	//step 3: change 'Length of UID/EPC' to EPC length of targetted RFID tag
	//step 4: Create 'Length' to length of data to write and starting address of 'input buffer'
	//step 5: Send command
	Rfid_changeMode(Idle, ch0_commandCode);
	std::this_thread::sleep_for(20ms);
	switch (channel) {
	case 0:
		Rfid_changeMemoryArea(userArea, ch0_memoryArea);
		std::this_thread::sleep_for(50ms);
		memcpy(awRFID_output, awEPC[rfidIndex], epcbyteLen); //copy EPC of target tag
		Rfid_changeEpcLength(epcbyteLen, ch0_EPClength); //length of target epc
		memcpy(awRFID_output + (epcbyteLen / 2), &writeByteData, 4); //insert data after EPC
		Rfid_changeOutputData((epcbyteLen + 4) / 2, ch0_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)4, ch0_length);//write doubleword
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch0_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(Write, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		break;
	case 1:
		Rfid_changeMemoryArea(userArea, ch1_memoryArea);
		std::this_thread::sleep_for(50ms);
		memcpy(awRFID_output, awEPC[rfidIndex], epcbyteLen); //copy EPC of target tag
		Rfid_changeEpcLength(epcbyteLen, ch1_EPClength); //length of target epc
		memcpy(awRFID_output + (epcbyteLen / 2), &writeByteData, 4); //insert data after EPC
		Rfid_changeOutputData((epcbyteLen + 4) / 2, ch1_outputData);
		std::this_thread::sleep_for(50ms);
		Rfid_changeByteLength((uint16_t)4, ch1_length);//write doubleword
		std::this_thread::sleep_for(50ms);
		Rfid_changeStartAddr((uint16_t)0, ch1_startAddr);
		std::this_thread::sleep_for(50ms);
		Rfid_changeMode(Write, ch1_commandCode);
		std::this_thread::sleep_for(50ms);
		break;
	}
	return 0;

}

int RfidTben::Rfid_initAllTags(int channel, int epcbyteLen, int scanTime) {
	uint32_t currentValue;
	Rfid_scanEPC(channel, scanTime);
	Rfid_parseEPCDetected(channel);
	for (int i = 0; i < wTagCounter; i++) {
		Rfid_WriteData(channel, i, EXPECTED_EPC_LENGTH, 0);
		currentValue = Rfid_readData(channel, i, EXPECTED_EPC_LENGTH);
		printf("TagID%d: %ld\n", i, currentValue);
	}
	return 0;
}

int RfidTben::Rfid_incrementAllTags(int channel, int epcbyteLen, int scanTime) {
	uint32_t previousValue;
	Rfid_scanEPC(channel, scanTime);
	Rfid_parseEPCDetected(channel);
	for (int i = 0; i < wTagCounter; i++) {
		previousValue = Rfid_readData(channel, i, EXPECTED_EPC_LENGTH);
		Rfid_WriteData(channel, i, EXPECTED_EPC_LENGTH, previousValue + 1);
		printf("TagID%d: %ld -> %ld\n", i, previousValue, previousValue + 1);
	}
	return 0;
}

void RfidTben::disconnectModbus() {
	modbus_close(modbusHandler);
	modbus_free(modbusHandler);
	printf("Closing modbus connection\n");
	return;
}

RfidTben::~RfidTben() {
	modbus_close(modbusHandler);
	modbus_free(modbusHandler);
	printf("Closing modbus connection\n");
}

//test function with hardcoding
int RfidTben::Rfid_changeEPCLength(int testmode) {
	int stop;
	//uint8_t input[14] = { 1,2,3,4,5,6,7,8,9,0,1,2,3,4 };
	uint16_t dinput[7] = { 57,56,55,54,53,52,51 };
	uint16_t input[7] = { 51,52,53,54,55,56,57 };
	
	Rfid_changeMemoryArea(EPC, ch0_memoryArea);
	std::this_thread::sleep_for(50ms);
	Rfid_changeMode(Idle, ch0_commandCode);
	std::this_thread::sleep_for(50ms);
	Rfid_changeStartAddr((uint16_t)0, ch0_startAddr); //readheadaddress?
	std::this_thread::sleep_for(50ms);
	switch (testmode) {

	case 0: //epclength 6 to 7
		Rfid_changeByteLength((uint16_t)14, ch0_length); //new epclength
		std::this_thread::sleep_for(50ms);
		//copy old epc
		memcpy(awRFID_output, awRFID_input + 1, 12); 
		Rfid_changeEpcLength(12, ch0_EPClength); //length of target epc
		//copy new epc
		memcpy(awRFID_output + 6, input, 14);
		Rfid_changeOutputData((6 + 7), ch0_outputData);
		std::this_thread::sleep_for(50ms);
		printf("awRFID_output: %d  %d %d %d %d %d\n ",
			awRFID_output[0], awRFID_output[1], awRFID_output[2], awRFID_output[3],
			awRFID_output[4], awRFID_output[5]);
		printf("awRFID_output 2: %d  %d %d %d %d %d %d %d\nPlease enter a NUMBER to continue: ",
			awRFID_output[6], awRFID_output[7], awRFID_output[8], awRFID_output[9],
			awRFID_output[10], awRFID_output[11], awRFID_output[12], awRFID_output[13]);
		scanf("%d", &stop);
		Rfid_changeMode(ChangeEPCLength, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		break;
	case 1: //epclength 7 to 6
		Rfid_changeByteLength((uint16_t)12, ch0_length);//new epclength
		std::this_thread::sleep_for(50ms);
		//copy old epc
		memcpy(awRFID_output, awRFID_input + 1, 14); 
		Rfid_changeEpcLength(12, ch1_EPClength); //length of target epc
		//copy new epc
		memcpy(awRFID_output + 7, input, 12); 
		Rfid_changeOutputData((6 + 7), ch0_outputData);
		std::this_thread::sleep_for(50ms);
		printf("awRFID_output: %d  %d %d %d %d %d\n ",
			awRFID_output[0], awRFID_output[1], awRFID_output[2], awRFID_output[3],
			awRFID_output[4], awRFID_output[5]);
		printf("awRFID_output 2: %d  %d %d %d %d %d %d %d\nPlease enter a NUMBER to continue: ",
			awRFID_output[6], awRFID_output[7], awRFID_output[8], awRFID_output[9],
			awRFID_output[10], awRFID_output[11], awRFID_output[12], awRFID_output[13]);
		scanf("%d", &stop);
		Rfid_changeMode(ChangeEPCLength, ch0_commandCode);
		std::this_thread::sleep_for(50ms);
		break;
	default:
		break;
	}
	return 0;
}

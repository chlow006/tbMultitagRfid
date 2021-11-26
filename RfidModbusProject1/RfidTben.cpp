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

int  RfidTben::readModbusInput(uint16_t startAddr, uint16_t numByteToRead) {
	int rc = 0;
	rc = modbus_read_registers(modbusHandler, startAddr, numByteToRead, awRFID_input);
	if (rc == -1) {
		fprintf(stderr, "read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	
	return rc;
}

int  RfidTben::writeModbus(uint16_t startAddr, int valueToWrite) {
	int wc = 0;
	wc = modbus_write_register(modbusHandler, startAddr, valueToWrite);
	if (wc == -1) {
		fprintf(stderr, "\twrit failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	
	return wc;
}

int RfidTben::Rfid_changeMode(RfidCommand command) {
	return writeModbus(0x800, command);
}

int RfidTben::Rfid_changeStartAddr(uint16_t addr) {

	return writeModbus(0x802, addr);
}

int RfidTben::Rfid_changeStartAddr(uint32_t addr) { //to be tested
	int wc;
	uint16_t temp[2];
	memcpy(temp, &addr, 2);
	wc = writeModbus(0x802, temp[0]);
	wc = writeModbus(0x803, temp[1]);
	return wc;
}

int RfidTben::Rfid_changeByteLength(uint16_t len) {
	
	return writeModbus(0x804, len);
}

int RfidTben::Rfid_changeByteLength(uint32_t len) { //to be tested
	int wc;
	uint16_t temp[2];
	memcpy(temp, &len, 2);
	wc = writeModbus(0x804, temp[0]);
	wc = writeModbus(0x805, temp[1]);
	return wc;
}

int RfidTben::Rfid_readTagInput(uint16_t len) {
	int rc = readModbusInput(0x0B, len);
	return rc;
}

int RfidTben::Rfid_readTagCounter() {
	int tc = modbus_read_registers(modbusHandler, 0x05, 1, &wTagCounter);
	if (tc == -1) {
		fprintf(stderr, "read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	return wTagCounter;
}

uint16_t RfidTben::Rfid_readByteAvailable() {
	int tc = modbus_read_registers(modbusHandler, 0x06, 1, &wByteAvailable);
	if (tc == -1) {
		fprintf(stderr, "read failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	printf("inside bytes available %d\n", wByteAvailable);
	return wByteAvailable;
}

int RfidTben::Rfid_scanTag() {
	//Flow chart: Continuous mode without interruption before reading data (TBEN-S2-2RFID-4DXP Manual page 270)
	//step 1: Set group address to detect unique IDs(Leaving it at zero will cause hardware to keep polling old tags)
	//step 2: Set length 
	//step 3: Start polling in continuous mode
	//step 4: Stop polling in continuous mode
	//step 5: Retrieve data

	Rfid_changeStartAddr((uint16_t)1);
	std::this_thread::sleep_for(200ms);
	Rfid_changeByteLength((uint16_t)16);
	std::this_thread::sleep_for(200ms);
	Rfid_changeMode(RfidTben::StartContinousMode);
	std::this_thread::sleep_for(200ms);
	Rfid_changeMode(RfidTben::StopContinousMode);
	std::this_thread::sleep_for(200ms);
	Rfid_changeByteLength(Rfid_readByteAvailable());
	std::this_thread::sleep_for(200ms);
	Rfid_changeMode(RfidTben::GetData);
	std::this_thread::sleep_for(200ms);
	return Rfid_readTagInput(wByteAvailable/2);
	
}

string RfidTben::wordToAscii(uint16_t wordSrc) {
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

int RfidTben::Rfid_parseTagDetected() {
	//assume rfid is a 12byte hex number.\
	//every rfid will readout will consist of the 16byte in total
	//byte 1: number of byte to follow
	//byte 2:
	//byte 3-14: EPC number
	//byte 15-16: 

	string str="";
	for (int i = 0; i < wTagCounter; i++) {
		str = "";
		for (int j = 0; j < 6; j++) {
			int index = 8*i + 2 + j ;
			str+=wordToAscii(awRFID_input[index]);
		}
		printf("%i: %s\n",i, str.c_str());
	}
	return 0;
}

RfidTben::~RfidTben() {
	modbus_close(modbusHandler);
	modbus_free(modbusHandler);
	printf("Closing modbus connection\n");
}
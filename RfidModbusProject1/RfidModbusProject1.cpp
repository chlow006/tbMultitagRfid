#include <iostream>
#include "RfidTben.h"
#include <Windows.h>

extern "C" {
#include <modbus.h>
}


int main()
{
	RfidTben mock1;
	mock1.connectModbus("192.168.1.13");
	int tc = mock1.Rfid_readTagCounter();	
	printf("read %d tags\n", tc);

	int rc = mock1.Rfid_scanTag();
	/*for (int i = 0; i < rc; i++) {
		printf("read %d registers reg[%d]=%d (0x%X)\n", rc, i, mock1.awRFID_input[i], mock1.awRFID_input[i]);
	}*/
	mock1.Rfid_parseTagDetected();

	
}

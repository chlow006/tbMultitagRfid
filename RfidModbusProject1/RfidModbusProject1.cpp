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
	
	int rc = mock1.Rfid_scanTag(0,2000);

	/*for (int i = 0; i < rc; i++) {
		printf("read %d registers reg[%d]=%d (0x%X)\n", rc, i, mock1.awRFID_input[i], mock1.awRFID_input[i]);
	}*/
	mock1.Rfid_parseTagDetected(0);
	printf("read %d tags\n", mock1.wTagCounter);
	
}

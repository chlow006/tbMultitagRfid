// RfidModbusProject1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <modbus.h>

int main()
{
    std::cout << "Hello World!\n";
	modbus_t * mb, *mb2;
	uint16_t tab_reg[32];
	int rc = 0, wc = 0;

	mb = modbus_new_tcp("192.168.1.2", 502);
	modbus_connect(mb);
	// Read 5 registers from the address 0 
	rc = modbus_read_registers(mb, 0, 5, tab_reg);

	for (int i = 0; i < rc; i++) {
		printf("read %d registers reg[%d]=%d (0x%X)\n", rc, i, tab_reg[i], tab_reg[i]);
	}
	modbus_close(mb);
	modbus_free(mb);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

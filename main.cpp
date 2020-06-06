

#include "View.h"
#include "Eye.h"
#include "Pixel.h"
#include <vector>
#include "struct.h"
#include <windows.h>
#include <stdio.h>

#include <string>
#include <time.h>
#include <iostream>

unsigned int available(HANDLE *serialHandle) {
	DWORD errors;
	COMSTAT status;
	ClearCommError(serialHandle, &errors, &status);
	return status.cbInQue;
}

int main() {
	/*View* m = new View();
	if (m->isInit) {
		Pixel* p = new Pixel(m->left->width, m->left->height);
		p->createZone(6);
		m->left->load();
		p->debugImage("left1.dump", m->left, false);
		m->right->load();
		p->debugImage("right1.dump", m->right, true);
		while (1) {
			m->left->load();
			p->getData(m->left, false);
			p->debugImage("left1.dump", m->left, false);
			Sleep(500);
			printf("---------\n");
		}
	}*/

	HANDLE serialHandle;
	BOOL status;

	serialHandle = CreateFileA("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	// Do some basic settings
	DCB serialParams = { 0 };
	status = serialParams.DCBlength = sizeof(serialParams);

	GetCommState(serialHandle, &serialParams);
	if (status == FALSE)
		printf("\n   Error! in GetCommState()");

	serialParams.BaudRate = 9600;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;
	serialParams.fDtrControl = DTR_CONTROL_ENABLE;

	status = SetCommState(serialHandle, &serialParams);
	if (status == FALSE)
		printf("\n   Error! in SetCommState()");

	// Set timeouts
	/*COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 50;
	timeout.ReadTotalTimeoutConstant = 50;
	timeout.ReadTotalTimeoutMultiplier = 50;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;

	status = SetCommTimeouts(serialHandle, &timeout);
	if (status == FALSE)
		printf("\n   Error! in SetCommTimeouts()");*/

	/*status = SetCommMask(serialHandle, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (status == FALSE)
		printf("\n\n    Error! in Setting CommMask");
	else
		printf("\n\n    Setting CommMask successfull");*/

	DWORD bytes_sent;
	DWORD dwEventMask;
	char  TempChar;                        // Temperory Character
	char  SerialBuffer[256];
	DWORD NoBytesRead;
	int x = 0;

	for (int i = 0; i < 100; i++) {
		status = WriteFile(serialHandle, "a", 1, &bytes_sent, NULL);
		if (status == FALSE) {
			printf("error writing");
		}
		printf("bits: %d\n", bytes_sent);
		Sleep(100);
		int bytesAvailable = available(&serialHandle);
		printf("available: %u\n", bytesAvailable);

		DWORD bytesRead;
		unsigned int toRead = 1;
		char buffer[] = { ' ' };
		if(bytesAvailable)
			if (ReadFile(serialHandle, buffer, toRead, &bytesRead, NULL))
				printf("char: %c\n", buffer[0]);
	}

	CloseHandle(serialHandle);
}

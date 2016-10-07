#include <windows.h>
#include <hhp.h>

HANDLE hSerial;

int32_t openSerial()
{
    LPCSTR portname = "\\\\.\\COM68";
    hSerial = CreateFile(portname,
            GENERIC_READ | GENERIC_WRITE,
            0,
            0,
            OPEN_EXISTING,
            0,
            0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Open Serial Port Failed, Invalid value: %d\r\n", GetLastError());
        return -1;
    }

    /* After opening the port further settings like Baudrate, Byte size, the number of stopbits and the Parity need to be set. */
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
         return -1;
    }
    dcbSerialParams.BaudRate=38400;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)){
         return -1;
    }

    /* Finally timeouts need to be set so that the program does not hang up when receiving nothing. */
    COMMTIMEOUTS timeouts={0};
    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(hSerial, &timeouts)){
        return -1;
    }
    return 0;
}

DWORD readFromSerialPort(uint8_t *data)
{
    DWORD dwBytesRead = 0;
    if (!ReadFile(hSerial, data, 1, &dwBytesRead, NULL)){
        //handle error
    }
    return dwBytesRead;
}

DWORD writeToSerialPort(uint8_t data)
{
	DWORD dwBytesRead = 0;
	if (!WriteFile(hSerial, &data, 1, &dwBytesRead, NULL)){
		//printLastError();
	}
	return dwBytesRead;

}

void closeSerialPort(HANDLE hSerial)
{
	CloseHandle(hSerial);
}

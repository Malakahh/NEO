#ifndef BLUETOOTH_H
#define BLUETOOTH_H

//Setup
#define BT_UART_BaudRate 115200

//Commands
#define BT_GetFirmwareVersion	"V\r"
#define BT_SetDeviceName		"SN\r"


// RN4020

void BTInit();
void BTSendCommand(char *cmd);
unsigned int BTGetResponse(char *buffer, unsigned int bufferSize);
void BTReboot();

#endif
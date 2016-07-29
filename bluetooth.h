#ifndef BLUETOOTH_H
#define BLUETOOTH_H

//Setup
#define BT_UART_Baud_Default 115200
#define BT_UART_Baud 2400

//Commands
#define BT_GetFirmwareVersion	"V\r"
#define BT_SetDeviceName		"SN\r"

//Feature settings
#define BT_FEATURE_DEFAULT 					0x00000000
#define BT_FEATURE_NO_DIRECT_ADVERTISEMENT 	0x04000000
#define BT_FEATURE_ENABLE_AUTHENTICATION	0x00400000

#define BTBufferSize 255


char BTbuffer[BTBufferSize];
char *BTbufferWriteItr = BTbuffer;
char *BTbufferReadItr = BTbuffer;

// RN4020

void BTInit();
void BTSendCommand(char *cmd);
void BTGetResponse(char *buffer);
void BTReboot();
void BTFactoryReset();
void BTUart1ClearBuffer();
void BTCmdMode(char enter);

unsigned long GetStoredBaud();
void SetStoredBaud(unsigned long baud);

void BTRelayResponse();

#endif
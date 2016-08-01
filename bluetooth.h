#ifndef BLUETOOTH_H
#define BLUETOOTH_H

//Setup
#define BT_UART_Baud_Default 115200
#define BT_UART_Baud 2400

#define BTBufferSize 255

#define UNDIRECTED_ADVERTISEMENT_TIME 0x0A

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

//void FirmwareUpgradeSetup();
void BTInterruptHandle();

void StartDirectedAdvertisement();
void StartUndirectedAdvertisement();

#endif
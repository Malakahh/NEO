#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "events.h"

//Setup
#define BT_UART_Baud_Default 115200
#define BT_UART_Baud 2400

#define BTBufferSize 255

#define UNDIRECTED_ADVERTISEMENT_TIME 60

// RN4020

void BTInit();
void BTSendCommand(char *cmd);

void BTReboot();
void BTFactoryReset();
void BTUart1ClearBuffer();
void BTCmdMode(char enter);
void BTUart1ClearBuffer();

unsigned long GetStoredBaud();
void SetStoredBaud(unsigned long baud);

void BTRelayResponse();

//void FirmwareUpgradeSetup();
void BTInterruptHandle();

void StartDirectedAdvertisement();
void StartUndirectedAdvertisement();

void BTByteUART1ToBuffer();
void BTByteBufferToUART2();

char BTBufferReadFromEnd(char* msg, char maxLength, char stopChar);
void StrReverse(char* str, char length);
char BTFindInBuffer(char *msg, char msgLength, char searchLength);

#endif
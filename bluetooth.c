#include "bluetooth.h"

#define InitBufferSize 255

void BTInit()
{
	char buffer[InitBufferSize];

	UART1_Init(BT_UART_BaudRate);
	Delay_ms(100);

	LATE.RE1 = 1;
	Delay_ms(100);
	/*UART_Set_Active(&UART1_Read, &UART1_Write, &UART1_Data_Ready, &UART1_Tx_Idle); 
	Delay_ms(100);*/

	//;
	//Delay_ms(150);
	if (BTGetResponse(buffer, InitBufferSize) > 0 && strcmp("CMD", buffer))
	{
		LATB.RB0 = 1;
		BTSendCommand("s-,NEO\r");
		LATB.RB1 = 1;
	}

	BTReboot();
}

void BTSendCommand(char *cmd)
{
	UART1_Write_Text(cmd);
}

unsigned int BTGetResponse(char *buffer, unsigned int bufferSize)
{
	unsigned int byteCount = 0;

	while (UART1_Data_Ready())
	{
		*buffer++ = UART1_Read();
		byteCount++;
	}

	return byteCount;
}

void BTReboot()
{
	BTSendCommand("r,1\r");
}
#include "bluetooth.h"
#include "events.h"

char BTbuffer[BTBufferSize];
char *BTbufferWriteItr = BTbuffer;
char *BTbufferReadItr = BTbuffer;



/*
void FirmwareUpgradeSetup()
{
    BTSendCommand("+\r");           //Echo on
    BTSendCommand("sf,2\r");        //Complete factory reset
    BTSendCommand("r,1\r");
    Delay_ms(250);
    BTSendCommand("sr,10008000\r"); //Support MLDP, enable OTA (peripheral mode is enabled by default)
    BTSendCommand("r,1\r");         //reboot to apply settings

    //Wait for "CMD"
    Delay_ms(2500);

    BTSendCommand("a\r");           //Start advertising
}
*/

unsigned long GetStoredBaud()
{
	int i,k;
	unsigned long res = 0;

	for (i = 0; i < 4; i++)
	{
		res = res | ((unsigned long)EEPROM_Read(i) << i * 8);
		Delay_ms(20);
	}

	return res;
}

void SetStoredBaud(unsigned long baud)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		EEPROM_Write(i, (baud & (((unsigned long)0xFF) << i * 8)) >> i * 8);
		Delay_ms(20);
	}
}

void StartDirectedAdvertisement()
{
	BTSendCommand("sr,20060000\r"); // no pin code
	BTReboot();
	//BTSendCommand("a\r");
}

void StartUndirectedAdvertisement()
{
	BTSendCommand("sr,24060000\r"); //No_Direct_Advertisement & no pin code
	BTReboot();
	//BTSendCommand("a\r");
}

void BTInit()
{
	unsigned long previousBaud;

	//Initiate BT buffer
	memset(BTbuffer, 0xFF, BTBufferSize);

	UART2_Init(BT_UART_Baud);
	Delay_ms(100);

	previousBaud = GetStoredBaud();
	if (previousBaud == BT_UART_Baud_Default || previousBaud == 0xFFFFFFFF)
	{
		//Setup UART interfaces - UART1 is the BT module, UART2 is the connected device (PC, smartphone, etc)
		UART1_Init(BT_UART_Baud_Default);
		Delay_ms(100);

		//Enter BT device into CMD mode
		BTCmdMode(1);

		//Set BT device to communicate with a baud of 2400
		//Only relevant when UART1's baud is not 2400
		BTSendCommand("sb,0\r");

		BTReboot();

		//CMD mode pin is no longer needed at 2400 baud, so turn it off.
		BTCmdMode(0);

		//Set baud for next boot
		SetStoredBaud(BT_UART_Baud);
	}

	UART1_Init(BT_UART_Baud);
	Delay_ms(100);

	BTSendCommand("s-,NEO\r");

	//BTFactoryReset();

	BTReboot();

	//BTSendCommand("u\r");

	StartUndirectedAdvertisement();

	//BTSendCommand("gdf\r");

	
}

void BTSendCommand(char *cmd)
{
	UART1_Write_Text(cmd);
}

void BTByteUART1ToBuffer()
{
	if (UART1_Data_Ready() == 1)
    {
        *BTbufferWriteItr++ = UART1_Read();
        if (BTbufferWriteItr >= BTbuffer + BTBufferSize)
        	BTbufferWriteItr = BTbuffer;
	}
}

void BTByteBufferToUART2()
{
	if (BTbufferReadItr != BTbufferWriteItr)
	{
		UART2_Write(*BTbufferReadItr++);
	    if (BTbufferReadItr >= BTbuffer + BTBufferSize)
	    	BTbufferReadItr = BTbuffer;
	}
}

char BTBufferReadFromEnd(char* msg, char maxLength, char stopChar)
{
	char length;

	for (length = 0; length < maxLength; length++)
	{
		*(msg + length) = *(BTbufferWriteItr - 1 - length);

		if (*(BTbufferWriteItr - 1 - length) == stopChar)
			break;
	}
	
	StrReverse(msg, length);
	return length;
}

char BTFindInBuffer(char *msg, char msgLength, char searchLength)
{
	char i, n;
	char temp;

	for (i = 0; i < searchLength; i++)
	{
		temp = 1;

		for (n = 0; n < msgLength; n++)
		{
			if (*(BTbufferWriteItr - 1 - i + n) != *(msg + n))
			{
				temp = 0;
			}
		}

		if (temp)
			return 1;
	}

	return 0;
}

void StrReverse(char* str, char length)
{
	char i, temp;

	for(i = 0; i < length / 2; i++)
	{
	    temp = str[i];
	    str[i] = str[length - i - 1];
	    str[length - i - 1] = temp;
	}
}

void BTRelayResponse()
{
	BTByteUART1ToBuffer();
	BTByteBufferToUART2();
}

void BTUart1ClearBuffer()
{
	if (UART1_Data_Ready() == 1)
		UART1_Read();

	if (UART1_Data_Ready() == 1)
		UART1_Read();
}

void BTReboot()
{
	BTSendCommand("r,1\r");
	Delay_ms(2000);
}

void BTFactoryReset()
{
	SetStoredBaud(0xFFFFFFFF);
	BTSendCommand("sf,2\r");
	Delay_ms(50);
	BTReboot();
	while(1);
}

void BTCmdMode(char enter)
{
	if (enter)
	{
		LATE.RE1 = 1;
	}
	else
	{
		LATE.RE1 = 0;
	}

	Delay_ms(150);
}
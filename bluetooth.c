#include "bluetooth.h"



void interrupt(void)
{
    if(RC1IF_bit == 1)
    {
        //BTRelayResponse();
        if (UART1_Data_Ready() == 1)
	    {
	    	//BTUart1ClearBuffer();
	        //UART2_Write(UART1_Read());
	        *BTbufferWriteItr++ = UART1_Read();
	        if (BTbufferWriteItr >= BTbuffer + BTBufferSize)
	        	BTbufferWriteItr = BTbuffer;

	        UART2_Write(*BTbufferReadItr++);
	        if (BTbufferReadItr >= BTbuffer + BTBufferSize)
	        	BTbufferReadItr = BTbuffer;
	    }
    }


}

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

void BTInit()
{
	//Declare variables
	
	//char msg[12];
	unsigned long previousBaud;
	int i;

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
	
	//BTFactoryReset();

	BTSendCommand("sr,04060000\r"); //No_Direct_Advertisement

	BTSendCommand("s-,NEO\r");
	
	BTReboot();

	BTSendCommand("a\r");

	BTSendCommand("gdf\r");

	LATB.RB1 = 1;
	//LATE.RE1 = 1;
	//Delay_ms(50);
	//LATE.RE1 = 1;


	
/*
	BTGetResponse(buffer); //Returns 'CM'

	//UART_Set_Active(&UART2_Read, &UART2_Write, &UART2_Data_Ready, &UART2_Tx_Idle);
	for (i = 0; i < InitBufferSize; i++)
	{
		UART2_Write(buffer[i]);
	}
*/


/*
	if (BTGetResponse(buffer) > 0)
	{
		Delay_ms(1000);
		LATE.RE1 = 0;
		Delay_ms(200);
		BTGetResponse(buffer2, InitBufferSize);

		


		//Change BT device name
		BTSendCommand("s-,NEO\r");

		//Get supported feature settings		
		BTSendCommand("gr");
		Delay_ms(100);
		if (BTGetResponse(buffer, InitBufferSize) > 0)
		{
			LATD.RD0 = 1;
			

			//buffer[1] = 0x06;

			//BTSendCommand(strcat(strcat("sr,", buffer), "\r"));
		}
	}

	BTReboot();
	*/
}

void BTSendCommand(char *cmd)
{
	UART1_Write_Text(cmd);
}

void BTGetResponse(char *buffer)
{
	Delay_ms(3);
	while (UART1_Data_Ready())
	{
		*buffer++ = UART1_Read();
	}
}

void BTRelayResponse()
{
	if (UART1_Data_Ready() == 1)
    {
        UART2_Write(UART1_Read());
    }
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
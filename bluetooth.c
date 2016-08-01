#include "bluetooth.h"


char BTbuffer[BTBufferSize];
char *BTbufferWriteItr = BTbuffer;
char *BTbufferReadItr = BTbuffer;
int time = 0;

void BTInterruptHandle()
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
    
    if (INTCON.TMR0IF == 1)
    {
		LATB.RB2 = !LATB.RB2;

		time++;
	    INTCON.TMR0IF = 0;

		if (time == UNDIRECTED_ADVERTISEMENT_TIME)
		{
			UART2_Write(time);
			T0CON.TMR0ON = 0;
			LATB.RB3 = 1;
			//StartDirectedAdvertisement();
		}
    } 
}

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
	BTSendCommand("y\r");
	BTSendCommand("sr,00060000\r"); // no pin code
	BTReboot();
	BTSendCommand("a\r");
}

void StartUndirectedAdvertisement()
{
	BTSendCommand("y\r");
	BTSendCommand("sr,04060000\r"); //No_Direct_Advertisement & no pin code
	BTReboot();
	BTSendCommand("a\r");
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
	
	StartUndirectedAdvertisement();

	//BTSendCommand("gdf\r");
	LATB.RB1 = 1;

	//Start timer
	T0CON.TMR0ON = 1;
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
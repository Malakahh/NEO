#include "bluetooth.h"
#include "events.h"

char directedAdvertisement = 1;

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
	if (directedAdvertisement == 0)
	{
		BTSendCommand("sr,20060000\r"); // no pin code & autoadvertise
		BTReboot();
	
		directedAdvertisement = 1;
	}
}

void StartUndirectedAdvertisement()
{
	if (directedAdvertisement == 1)
	{
		BTSendCommand("sr,24060000\r"); //No_Direct_Advertisement & no pin code & autoadvertise
		BTReboot();

		directedAdvertisement = 0;
	}
}

void SetupPrivateServices()
{
	BTSendCommand("SS,00000001\r"); 	//Enable private service support
	BTSendCommand("PZ\r");			//Clear current private service and characteristics
	BTSendCommand("PS,f4f232be5a5311e68b7786f30ca893d3\r"); //Set private service UUID to be 0xf4f232be5a5311e68b7786f30ca893d3

	//Add private characteristic 0x1d4b745a5a5411e68b7786f30ca893d3 to
	//current private service. The property of this characterstic is 0x02
	//(readable) and has a maximum data size of 0x14 (20 bytes).
	BTSendCommand("PC,1d4b745a5a5411e68b7786f30ca893d3,02,14\r");

	//Add private characteristic 0xe25328b05a5411e68b7786f30ca893d3 to
	//current private service. The property of this cahracteristic is 0x18,
	//(writable and could notify) and has a maximum data size of 0x14 (20 bytes).
	BTSendCommand("PC,e25328b05a5411e68b7786f30ca893d3,18,14\r");
}	

void BTInit()
{
	unsigned long previousBaud;

	UART2_Init(BT_UART_Baud);
	Delay_ms(100);

	previousBaud = GetStoredBaud();
	if (previousBaud == BT_UART_Baud_Default || previousBaud == 0xFFFFFFFF)
	{
		LATB.RB3 = 1;

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

	BTSendCommand("s-,NEO\r");

	SetupPrivateServices();

	BTReboot();

	StartUndirectedAdvertisement();


	//BTSendCommand("gdf\r");

	
}

void BTSendCommand(char *cmd)
{
	UART1_Write_Text(cmd);
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
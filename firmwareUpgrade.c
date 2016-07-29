#include "bluetooth.h"



void FirmwareUpgradeSetup()
{
    unsigned long previousBaud;

    previousBaud = GetStoredBaud();
    if (previousBaud == BT_UART_Baud_Default || previousBaud == 0xFFFFFFFF)
    {
        //Setup UART interfaces - UART1 is the BT module, UART2 is the connected device (PC, smartphone, etc)
        UART1_Init(BT_UART_Baud_Default);
        Delay_ms(100);
    }
    else
    {
        UART1_Init(BT_UART_Baud);
        Delay_ms(100);
    }

    BTSendCommand("+\r");           //Echo on
    BTFactoryReset();               //Complete factory reset
    BTSendCommand("sr,10008000\r"); //Support MLDP, enable OTA (peripheral mode is enabled by default)
    BTReboot();                     //reboot to apply settings

    //Wait for "CMD"
    while (!(*(BTbufferReadItr - 4) == 'C' && *(BTbufferReadItr - 3) == 'M' && *(BTbufferReadItr - 2) == 'D'));

    BTSendCommand("a\r");           //Start advertising
}
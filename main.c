#include "bluetooth.h"
#include "firmwareUpgrade.c"

void InitPorts()
{
	ANSELC = 0;
    ANSELD = 0;

    //TRISA = 0;
    TRISB = 0;
    //TRISC = 0;
    //TRISD = 0;
    TRISE = 0;

    //LATA = 0x00;
    LATB = 0x00;
    //LATC = 0x00;
    //LATD = 0x00;
    LATE = 0x00;
}

void main() {
    InitPorts();

    RC1IE_bit = 1;  // turn ON interrupt on UART1 receive
    RC1IF_bit = 0;  // Clear interrupt flag
    PEIE_bit  = 1;  // Enable peripheral interrupts
    GIE_bit   = 1;  // Enable GLOBAL interrupts

	BTInit();
}
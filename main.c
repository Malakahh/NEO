#include "bluetooth.h"

void interrupt()
{
    BTInterruptHandle();
}

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

void InitInterrupts()
{
    RC1IE_bit = 1;  // turn ON interrupt on UART1 receive
    RC1IF_bit = 0;  // Clear interrupt flag
    PEIE_bit  = 1;  // Enable peripheral interrupts
    GIE_bit   = 1;  // Enable GLOBAL interrupts


    //Interrupt timer, 1Hz
    INTCON.TMR0IE = 1;      // Enable timer0
    T0CON.T08BIT = 0;       // Timer is now 16bit
    T0CON.T0CS = 0;         // Internal clock
    T0CON.PSA = 0;          
    T0CON.T0PS2 = 1;
    T0CON.T0PS1 = 0;
    T0CON.T0PS0 = 0;
    TMR0H = 0xB;
    TMR0L = 0xDC;
}

void main() {
    InitPorts();
    InitInterrupts();

	BTInit();
}
#include "bluetooth.h"
#include "events.h"

#define MAX_MSG_LENGTH 25

int time = 0;

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
    PEIE_bit  = 1;  // Enable peripheral interrupts
    GIE_bit   = 1;  // Enable GLOBAL interrupts

    //UART1
    RC1IE_bit = 1;  // turn ON interrupt on UART1 receive
    RC1IF_bit = 0;  // Clear interrupt flag

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
    T0CON.TMR0ON = 0;       // Clear interrupt flag
}

void interrupt()
{
    if(RC1IF_bit == 1) //UART1 receive
    {
        QueueEvent(ON_UART1_RECEIVE);
        
        //Disable UART1 interrupt
        RC1IE_bit = 0;
    }

    if (INTCON.TMR0IF == 1)
    {
        time++;
        INTCON.TMR0IF = 0;
        LATB.RB4 = !LATB.RB4;

        if (time >= UNDIRECTED_ADVERTISEMENT_TIME)
        {
            QueueEvent(ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED);
        }
    } 

}

char StrCompare(char *str1, char length1, char *str2, char length2)
{
    char i;

    for (i = 0; i < length1; i++)
    {
        if (*(str1 + i) != *(str2 + i))
        {
            return 0;
        }
    }

    return length1 != length2;
}

void EventHandler(char event)
{

    if (event == ON_UART1_RECEIVE)
    {
        BTRelayResponse();

        //msgLength = BTBufferReadFromEnd(msg, MAX_MSG_LENGTH, '\n');
        if (BTFindInBuffer("Conn", 4, 30))
        {
            T0CON.TMR0ON = 0;
        }

        //Re-enable UART1 interrupt
        RC1IE_bit = 1;
    }
    else if (event == ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED)
    {
        UART2_Write(time);
        T0CON.TMR0ON = 0;
        LATB.RB1 = 1;
        
        StartDirectedAdvertisement();
    }
}

void main() {
    InitPorts();
    InitEvents();
    InitInterrupts();
	BTInit();
  
    //Start timer
    T0CON.TMR0ON = 1;

    while (1)
    {
        EventHandler(DequeueEvent());
    }
}
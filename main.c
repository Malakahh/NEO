#include "bluetooth.h"
#include "events.h"

#define UART1_BUFFER_SIZE 156

#define writeReg 0x80
#define readReg 0x00
#define writeEEprom 0x40
#define readEEprom 0x00
#define c_cmd_ee_data_high 0x05
#define c_cmd_ee_data_low 0x06
#define c_cmd_ee_addr_high 0x07
#define c_cmd_ee_addr_low 0x08

#define TerminalWrite Soft_UART_Write
#define ChargerWriteByte UART2_Write
#define ChargerRead UART2_Read
#define ChargerDataReady UART2_Data_Ready

char UART1Buffer[UART1_BUFFER_SIZE];
char *UART1BufferWriteItr = UART1Buffer;

int time = 0;
char connectionEstablished = 0;
char receivingChargerMsg = 0;

int test = 0;

void InitPorts()
{

	ANSELC = 0;
    ANSELB = 0;
    ANSELD = 0;

    //TRISA = 0;
    TRISB = 0;
    TRISC = 0x08;
    //TRISD = 0;
    TRISE = 0;


    //LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
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

    //UART2
    RC2IF_bit = 1;  // turn ON interrupt on UART2 receive
    RC2IF_bit = 0;  // Clear interrupt flag

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

    if (connectionEstablished == 0 && INTCON.TMR0IF == 1)
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

char ReadFromBT(char *output)
{
    if (UART1_Data_Ready() == 1)
    {
        *UART1BufferWriteItr++ = UART1_Read();
        *output = *(UART1BufferWriteItr - 1);

        //Bounds
        if (UART1BufferWriteItr >= UART1Buffer + UART1_BUFFER_SIZE)
            UART1BufferWriteItr = UART1Buffer;

        return 1;
    }

    return 0;
}

char FindInBuffer(char *msg, char msgLength, char searchLength)
{
    char searchItr, msgItr;
    char found;

    for (searchItr = 0; searchItr < searchLength - msgLength; searchItr++)
    {
        found = 1;

        for (msgItr = 0; msgItr < msgLength; msgItr++)
        {
            if (*(UART1BufferWriteItr - 1 - searchItr - msgLength + msgItr) != *(msg + msgItr))
            {
                found = 0;
            }
        }

        if (found)
            return 1;
    }

    return 0;
}

void EventHandler(char event)
{
    char *received;

    if (event == ON_UART1_RECEIVE)
    {
        ReadFromBT(received);
        UART2_Write(*received);

        if (receivingChargerMsg)
        {
            if (*received == '|')
            {
                receivingChargerMsg = 0;
            }

            BTSendCommand(sprinti("suw,1d4b745a5a5411e68b7786f30ca893d3,%c\r", *received));
        }
        else
        {
            if (*received == '\n')
            {
                if (connectionEstablished == 0 && FindInBuffer("Conn", 4, 15))
                {
                    T0CON.TMR0ON = 0;
                    connectionEstablished = 1;
                }
                else if (connectionEstablished == 1 && FindInBuffer("End", 3, 10))
                {
                    connectionEstablished = 0;
                    StartDirectedAdvertisement();
                }
            }
            else if (*received == '|')
            {
                receivingChargerMsg = 1;
            }
            else if (*received == '.')
            {
                if (FindInBuffer("747C", 4, 10))
                {
                    Delay_ms(5000);
                    BTSendCommand("sur,e25328b05a5411e68b7786f30ca893d3\r");
                }
            }
        }

        //Re-enable UART1 interrupt
        RC1IE_bit = 1;
    }
    else if (event == ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED)
    {
        T0CON.TMR0ON = 0;
        LATB.RB1 = 1;

        StartDirectedAdvertisement();
    }
}

void TerminalWriteText(char *msg)
{
    int i;
    for (i = 0; i < strlen(msg); i++)
    {
        TerminalWrite(msg[i]);
    }
}

void InitCharger()
{
    UART2_Init(1200);
    Delay_ms(100);
}

void InitTerminal()
{
    Soft_UART_Init(&LATC, 5, 4, 2400, 0);
    Delay_ms(100);
}

void ChargerTest()
{
    char error;
    char msg;
    int i = 0;

    InitPorts();
    InitTerminal();
    InitCharger();

    TerminalWriteText("Begin\n");

    ChargerWriteByte(c_cmd_ee_addr_high | writeReg);
    Delay_ms(100);
    ChargerWriteByte(0x00);
    Delay_ms(100);
    ChargerWriteByte(c_cmd_ee_addr_low | writeReg);
    Delay_ms(100);
    ChargerWriteByte(0x01);
    Delay_ms(100);
    ChargerWriteByte(c_cmd_ee_data_high | readReg);
    Delay_ms(100);

    LATB.RB0 = 1;

    while (1)
    {
        if (ChargerDataReady())
        {
            TerminalWrite(ChargerRead());
            break;
        }
    }

    ChargerWriteByte(c_cmd_ee_data_low | readReg);
    Delay_ms(100);

    LATB.RB2 = 1;

    while (1)
    {
        if (ChargerDataReady())
        {
            TerminalWrite(ChargerRead());
            break;
        }
    }

/*

    LATB.RB4 = 1;

    while(1)
    {
        msg = ChargerRead();

        if (msg != 0 && (i < 1 || msg != 0x5e))
        {
            TerminalWrite(msg);

            if (msg == 0x5e)
                i++;
        }

        LATB.RB5 = 1;
    }
    */
}

void main() {
    /*
    memset(UART1Buffer, 0xFF, UART1_BUFFER_SIZE);

    InitPorts();
    InitEvents();
    InitInterrupts();
    BTInit();

    //BTSendCommand("suw,1d4b745a5a5411e68b7786f30ca893d3,AAAABAAAABAAAABAAAAB\r");

    //Start timer
    T0CON.TMR0ON = 1;

    while (1)
    {
        EventHandler(DequeueEvent());
        test++;

        if (test > 100000)
        {
            test = 0;
            //BTSendCommand("sur,e25328b05a5411e68b7786f30ca893d3\r");
        }

    }
    */

    ChargerTest();
}
#include "bluetooth.h"
#include "events.h"

#define UART1_BUFFER_SIZE 100
#define UART2_BUFFER_SIZE 150

#define writeReg 0x80
#define readReg 0x00
#define writeEEprom 0x40
#define readEEprom 0x00
#define c_cmd_ee_data_high 0x05
#define c_cmd_ee_data_low 0x06
#define c_cmd_ee_addr_high 0x07
#define c_cmd_ee_addr_low 0x08

#define ChargerWriteByte UART2_Write

char UART1Buffer[UART1_BUFFER_SIZE];
char *UART1BufferWriteItr = UART1Buffer;
char *UART1BufferReadItr = UART1Buffer;

char UART2Buffer[UART2_BUFFER_SIZE];
char *UART2BufferWriteItr = UART2Buffer;
char *UART2BufferReadItr = UART2Buffer;

int time = 0;
char connectionEstablished = 0;
char hexParserByetCnt = 0;
char currentEventQueue = 1;
char relayToCharger = 0;

void TerminalWrite(char msg)
{
    RC1IE_bit = 0;
    RC2IE_bit = 0;

    Soft_UART_Write(msg);

    RC1IE_bit = 1;
    RC2IE_bit = 1;
}

void TerminalWriteText(char *msg)
{
    int i;

    RC1IE_bit = 0;
    RC2IE_bit = 0;

    for (i = 0; i < strlen(msg); i++)
    {
        TerminalWrite(msg[i]);
    }

    RC1IE_bit = 1;
    RC2IE_bit = 1;
}

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
    RC2IE_bit = 1;  // turn ON interrupt on UART2 receive
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

void WriteBuffer1()
{
    if (UART1_Data_Ready() == 1)
    {
        *UART1BufferWriteItr++ = UART1_Read();

        QueueEvent1(ON_UART1_RECEIVE);

        //Bounds
        if (UART1BufferWriteItr >= UART1Buffer + UART1_BUFFER_SIZE)
            UART1BufferWriteItr = UART1Buffer;
    }
}

void WriteBuffer2()
{
    if (UART2_Data_Ready() == 1)
    {
        *UART2BufferWriteItr++ = UART2_Read();

        QueueEvent2(ON_UART2_RECEIVE);

        //Bounds
        if (UART2BufferWriteItr >= UART2Buffer + UART2_BUFFER_SIZE)
            UART2BufferWriteItr = UART2Buffer;
    }
}

char ReadBuffer1()
{
    char ret = *UART1BufferReadItr++;
    
    if (UART1BufferReadItr >= UART1Buffer + UART1_BUFFER_SIZE)
    {
        UART1BufferReadItr = UART1Buffer;
    }

    return ret;
}

char ReadBuffer2()
{
    char ret = *UART2BufferReadItr++;
    
    if (UART2BufferReadItr >= UART2Buffer + UART2_BUFFER_SIZE)
    {
        UART2BufferReadItr = UART2Buffer;
    }

    return ret;
}

void interrupt()
{
    if (RC1IF_bit == 1) //UART1 receive
    {
        WriteBuffer1();
    }

    if (RC2IF_bit == 1) //UART2 receive
    {
        WriteBuffer2();
    }

    if (connectionEstablished == 0 && INTCON.TMR0IF == 1)
    {
        time++;
        INTCON.TMR0IF = 0;
        LATB.RB4 = !LATB.RB4;

        if (time >= UNDIRECTED_ADVERTISEMENT_TIME)
        {
            QueueEvent1(ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED);
        }
    } 
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
            if (*(UART1BufferReadItr - 1 - searchItr - msgLength + msgItr) != *(msg + msgItr))
            {
                found = 0;
            }
        }

        if (found)
            return 1;
    }

    return 0;
}

char ParseHex()
{
    char byte[2];

    if (UART1BufferReadItr == UART1Buffer)
    {
        byte[0] = *(UART1Buffer + UART1_BUFFER_SIZE - 2);
        byte[1] = *(UART1Buffer + UART1_BUFFER_SIZE - 1);
    }
    else if (UART1BufferReadItr == UART1Buffer + 1)
    {
        byte[0] = *(UART1Buffer + UART1_BUFFER_SIZE - 1);
        byte[1] = *(UART1BufferReadItr - 1);
    }
    else
    {
        byte[0] = *(UART1BufferReadItr - 2);
        byte[1] = *(UART1BufferReadItr - 1);   
    }

    return xtoi(byte);
}



void EventHandler1(char event)
{
    char received;
    char parsedHex;

    if (event == ON_UART1_RECEIVE)
    {
        received = ReadBuffer1();
        parsedHex = ParseHex();
        //TerminalWrite(received);

        if (parsedHex == '|')
        {
            relayToCharger = !relayToCharger;
            hexParserByetCnt = 0;
            return;
        }

        if (relayToCharger)
        {
            hexParserByetCnt++;

            if (hexParserByetCnt == 2)
            {
                hexParserByetCnt = 0;
                TerminalWrite(parsedHex);
                ChargerWriteByte(parsedHex);
                Delay_ms(15); //Per specification of the charger software
            }
        }
        else if (received == '\n')
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
    }
    else if (event == ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED)
    {
        T0CON.TMR0ON = 0;
        //LATB.RB1 = 1;

        StartDirectedAdvertisement();
    }
}

void EventHandler2(char event)
{
    char received;
    char buffer[60];

    if (event == ON_UART2_RECEIVE)
    {
        hexParserByetCnt = 0;
        relayToCharger = 0;
        received = ReadBuffer2();

        //LATB.RB7 = 1;
        TerminalWrite('\n');
        sprinti(buffer, "suw,1d4b745a5a5411e68b7786f30ca893d3,%02x\r", (unsigned int)received);
        TerminalWriteText(buffer);

        BTSendCommand(buffer);
        //BTSendCommand("suw,1d4b745a5a5411e68b7786f30ca893d3,AAAABAAAABAAAABAAAAB\r");

        //TerminalWrite(*received);

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

/*
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
}
*/

void main() {
    char event;

    memset(UART1Buffer, 0xFF, UART1_BUFFER_SIZE);

    InitPorts();
    InitEvents();
    InitInterrupts();
    InitTerminal();
    InitCharger();
    InitBT();

    //BTSendCommand("suw,1d4b745a5a5411e68b7786f30ca893d3,AAAABAAAABAAAABAAAAB\r");

    //Start timer
    T0CON.TMR0ON = 1;

    while (1)
    {
        if (currentEventQueue == 1)
        {
            event = DequeueEvent1();
            if (event == NO_EVENT)
            {
                currentEventQueue = 2;
                continue;
            }

            EventHandler1(event);
        }
        else if (currentEventQueue == 2)
        {
            event = DequeueEvent2();
            if (event == NO_EVENT)
            {
                currentEventQueue = 1;
                continue;
            }

            EventHandler2(event);
        }
        
    }
}
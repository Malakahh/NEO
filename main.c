/*

	Written for PIC18F25K22
	with Internal RC Oscillator

*/

#include "globals.h"
#include "bluetooth.h"
#include "events.h"

#define UART1_BUFFER_SIZE 150
#define UART2_BUFFER_SIZE 100

#define START_BYTE '|'
#define CHECKSUM_LENGTH_BYTES 4
#define DATA_MAX_LENGTH 4

#define ChargerWriteByte UART2_Write

typedef struct {
        char dat;
        char fromCharger;
} Response;

char UART1Buffer[UART1_BUFFER_SIZE];
char *UART1BufferWriteItr = UART1Buffer;
char *UART1BufferReadItr = UART1Buffer;

Response UART2Buffer[UART2_BUFFER_SIZE];
Response *UART2BufferWriteItr = UART2Buffer;
Response *UART2BufferReadItr = UART2Buffer;

char connectionEstablished = 0;
char hexParserByetCnt = 0;

signed char byteCntToRead = 0;
char readingChecksum = 0;
char checksum[CHECKSUM_LENGTH_BYTES];
char msgToRelay[DATA_MAX_LENGTH];
char *msgToRelayItr = msgToRelay;

//FOR DEBUGGING
void TerminalWrite(char msg)
{
    RC1IE_bit = 0;
    RC2IE_bit = 0;

    Soft_UART_Write(msg);

    RC1IE_bit = 1;
    RC2IE_bit = 1;
}

//FOR DEBUGGING
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
    ANSELB = 0;
    ANSELC = 0;

    //TRISB = 0x80;
    TRISC = 0x20;
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
}

void WriteBuffer1(char c)
{
        *UART1BufferWriteItr++ = c;

    QueueEvent(ON_UART1_RECEIVE);

    //Bounds
    if (UART1BufferWriteItr >= UART1Buffer + UART1_BUFFER_SIZE)
        UART1BufferWriteItr = UART1Buffer;
}

void WriteBuffer1FromUART()
{
    if (UART1_Data_Ready() == 1)
    {
        *UART1BufferWriteItr++ = UART1_Read();

            QueueEventFromUART(ON_UART1_RECEIVE);

            //Bounds
            if (UART1BufferWriteItr >= UART1Buffer + UART1_BUFFER_SIZE)
                UART1BufferWriteItr = UART1Buffer;
    }
}

void WriteBuffer2(char c)
{
        UART2BufferWriteItr->dat = c;
        UART2BufferWriteItr->fromCharger = 0;
        UART2BufferWriteItr++;

    QueueEvent(ON_UART2_RECEIVE);

    //Bounds
    if (UART2BufferWriteItr >= UART2Buffer + UART2_BUFFER_SIZE)
        UART2BufferWriteItr = UART2Buffer;
}

void WriteBuffer2FromUART()
{
    if (UART2_Data_Ready() == 1)
    {
        UART2BufferWriteItr->dat = UART2_Read();
        UART2BufferWriteItr->fromCharger = 1;
                UART2BufferWriteItr++;

            QueueEventFromUART(ON_UART2_RECEIVE);

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

Response* ReadBuffer2()
{
    Response* ret = UART2BufferReadItr;
    UART2BufferReadItr++;
    
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
        WriteBuffer1FromUART();
    }

    if (RC2IF_bit == 1) //UART2 receive
    {
        WriteBuffer2FromUART();
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
    memset(byte, 0x00, 2);

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

char ValidateChecksum()
{
        unsigned long newChkSum = CRC32_Tab(msgToRelay, byteCntToRead, -1);

        unsigned long control = (unsigned long)checksum[3];
        control = control | (unsigned long)checksum[0] << 8 * 3;
        control = control | (unsigned long)checksum[1] << 8 * 2;
        control = control | (unsigned long)checksum[2] << 8 * 1;

        if (newChkSum == control)
        {
                return 1;
        }
        else
        {
                return 0;
        }
}

void OnEvent_ON_UART1_RECEIVE()
{
        int i;
        char received = ReadBuffer1();
        char parsedHex = ParseHex();

        //TerminalWrite(received);

    if (byteCntToRead == 0 && parsedHex == START_BYTE) //New msg received
    {
            // TerminalWrite(parsedHex);
            // TerminalWrite('\n');

        byteCntToRead = -1;
        hexParserByetCnt = 0;
    }
    else if (byteCntToRead == -1) //Read byteCount
    {
            hexParserByetCnt++;

            if (hexParserByetCnt == 2)
        {
                    byteCntToRead = parsedHex;

                    //Reset stuff
                hexParserByetCnt = 0;
                    readingChecksum = 0;
                    msgToRelayItr = msgToRelay;

                    // TerminalWrite(byteCntToRead);
                    // TerminalWrite('\n');
            }
    }
    else if (msgToRelayItr < msgToRelay + byteCntToRead && byteCntToRead != 0) //Read msg
    {
        hexParserByetCnt++;
        
        if (hexParserByetCnt == 2)
        {
                if (readingChecksum < CHECKSUM_LENGTH_BYTES)
                {
                        //Reading checksum
                        checksum[readingChecksum++] = parsedHex;

                        //TerminalWrite(parsedHex);
                }
                else
                {
                        //Reading msg
                        *msgToRelayItr++ = parsedHex;

                        //Final byte of msg read
                        if (msgToRelayItr == msgToRelay + byteCntToRead)
                        {
                                if (ValidateChecksum() == 1)
                                {
                                        WriteBuffer2(START_BYTE);

                                        for (i = 0; msgToRelay + i < msgToRelayItr; i++)
                                        {
                                                ChargerWriteByte(msgToRelay[i]);
                                            Delay_ms(15); //Per specification of the charger software
                                        }
                                }

                                byteCntToRead = 0;
                        }
                }
                        
            hexParserByetCnt = 0;
        }
    }
    else if (received == '\n')
    {
        if (connectionEstablished == 0 && FindInBuffer("Conn", 4, 15))
        {
            connectionEstablished = 1;
        }
        else if (connectionEstablished == 1 && FindInBuffer("End", 3, 10))
        {
            connectionEstablished = 0;
            //StartDirectedAdvertisement();
            StartUndirectedAdvertisement();
        }
    }
}

void OnEvent_ON_UART2_RECEIVE()
{
        Response* received = ReadBuffer2();
    char buffer[49];
        memset(buffer, 0x00, 49);



        if (received->fromCharger == 0 && received->dat == START_BYTE)
        {
                sprinti(buffer, "suw,1d4b745a5a5411e68b7786f30ca893d3,%02x\r", (unsigned int)(received->dat));
        }
        else if (received->fromCharger == 1)
        {
                unsigned long newChkSum = CRC32_Tab(&(received->dat), 1, -1);

                sprinti(buffer, "suw,1d4b745a5a5411e68b7786f30ca893d3,%02x%02x%02x%02x%02x\r",
                        (unsigned int)((newChkSum >> 8 * 3) & 0xFF),
                        (unsigned int)((newChkSum >> 8 * 2) & 0xFF),
                        (unsigned int)((newChkSum >> 8 * 1) & 0xFF),
                        (unsigned int)(newChkSum & 0xFF),
                        (unsigned int)(received->dat));
        }    

        //TerminalWriteText(buffer);

        BTSendCommand(buffer);

    //Delay to allow for bluetooth notification to take place. Note that this is half the delay of UART1, due to possibly having to send two packets here.
    //Delay_ms(50);
}

void EventHandler(char event)
{
    if (event == ON_UART1_RECEIVE)
    {
        OnEvent_ON_UART1_RECEIVE();
    }
    else if (event == ON_UART2_RECEIVE)
    {
            OnEvent_ON_UART2_RECEIVE();
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

void main() {
    char event;

    //Setup internal oscillator
    OSCCON = 0x62;

    memset(UART1Buffer, 0xFF, UART1_BUFFER_SIZE);
    memset(UART2Buffer, 0xFF, UART2_BUFFER_SIZE);

    InitPorts();
    InitEvents();
    InitInterrupts();
    InitTerminal();
    InitCharger();
    InitBT();

    //Synchronie with charger to prevent corruption on power on
    ChargerWriteByte(0x3F);

    while (1)
    {
        EventHandler(DequeueEvent());
    }
}
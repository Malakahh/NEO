#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_BUFFER_SIZE 4

#define NO_EVENT 0x00
#define ON_UART1_RECEIVE 0x01
#define ON_UART2_RECEIVE 0x02

void InitEvents();

void QueueEvent(char event);
void QueueEventFromUART(char event);
char DequeueEvent();

#endif
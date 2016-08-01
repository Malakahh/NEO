#ifndef EVENTS_H
#define EVENTS_H

#define EVENT_BUFFER_SIZE 100

#define NO_EVENT 0x00
#define ON_UART1_RECEIVE 0x01
#define ON_UART2_RECEIVE 0x02
#define ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED 0x03

void InitEvents();
void QueueEvent(char event);
char DequeueEvent();

#endif
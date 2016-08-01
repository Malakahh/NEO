#include "events.h"

char eventBuffer[EVENT_BUFFER_SIZE];
char *eventBufferWriteItr;
char *eventBufferReadItr;

void InitEvents()
{
	memset(eventBuffer, 0xFF, EVENT_BUFFER_SIZE);
	eventBufferWriteItr = eventBuffer;
	eventBufferReadItr = eventBuffer;
}

void QueueEvent(char event)
{
	*eventBufferWriteItr++ = event;
    if (eventBufferWriteItr >= eventBuffer + EVENT_BUFFER_SIZE)
    	eventBufferWriteItr = eventBuffer;
}

char DequeueEvent()
{
	char event = NO_EVENT;
	if (eventBufferReadItr != eventBufferWriteItr)
	{
		event = *eventBufferReadItr++;
	    if (eventBufferReadItr >= eventBuffer + EVENT_BUFFER_SIZE)
	    	eventBufferReadItr = eventBuffer;
	}

    return event;
}
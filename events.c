#include "events.h"

char eventBuffer1[EVENT_BUFFER_SIZE];
char *eventBuffer1WriteItr;
char *eventBuffer1ReadItr;

char eventBuffer2[EVENT_BUFFER_SIZE];
char *eventBuffer2WriteItr;
char *eventBuffer2ReadItr;

void InitEvents()
{
	memset(eventBuffer1, 0xFF, EVENT_BUFFER_SIZE);
	eventBuffer1WriteItr = eventBuffer1;
	eventBuffer1ReadItr = eventBuffer1;

	memset(eventBuffer2, 0xFF, EVENT_BUFFER_SIZE);
	eventBuffer2WriteItr = eventBuffer2;
	eventBuffer2ReadItr = eventBuffer2;
}

void QueueEvent1(char event)
{
	*eventBuffer1WriteItr++ = event;
    if (eventBuffer1WriteItr >= eventBuffer1 + EVENT_BUFFER_SIZE)
    	eventBuffer1WriteItr = eventBuffer1;
}

void QueueEvent2(char event)
{
	*eventBuffer2WriteItr++ = event;
    if (eventBuffer2WriteItr >= eventBuffer2 + EVENT_BUFFER_SIZE)
    	eventBuffer2WriteItr = eventBuffer2;
}

char DequeueEvent1()
{
	char event = NO_EVENT;
	if (eventBuffer1ReadItr != eventBuffer1WriteItr)
	{
		event = *eventBuffer1ReadItr++;
	    if (eventBuffer1ReadItr >= eventBuffer1 + EVENT_BUFFER_SIZE)
	    	eventBuffer1ReadItr = eventBuffer1;
	}

    return event;
}

char DequeueEvent2()
{
	char event = NO_EVENT;
	if (eventBuffer2ReadItr != eventBuffer2WriteItr)
	{
		event = *eventBuffer2ReadItr++;
	    if (eventBuffer2ReadItr >= eventBuffer2 + EVENT_BUFFER_SIZE)
	    	eventBuffer2ReadItr = eventBuffer2;
	}

    return event;
}
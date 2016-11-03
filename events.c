#include "events.h"

char events[EVENT_BUFFER_SIZE];

void InitEvents()
{
	memset(events, 0, EVENT_BUFFER_SIZE);
}

void QueueEvent(char event)
{
	events[event]++;
}

void QueueEventFromUART(char event)
{
	events[event]++;
}

char DequeueEvent()
{
	if (events[ON_UART2_RECEIVE] > 0)
	{
		events[ON_UART2_RECEIVE]--;
		return ON_UART2_RECEIVE;
	}
	if (events[ON_UART1_RECEIVE] > 0)
	{
		events[ON_UART1_RECEIVE]--;
		return ON_UART1_RECEIVE;
	}
	else if (events[ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED] > 0)
	{
		events[ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED]--;
		return ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED;
	}
	else
	{
		return NO_EVENT;
	}
}
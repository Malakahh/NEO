#include "events.h"

char events1[EVENT_BUFFER_SIZE];
char events2[EVENT_BUFFER_SIZE];

void InitEvents()
{
	memset(events1, 0, EVENT_BUFFER_SIZE);
	memset(events2, 0, EVENT_BUFFER_SIZE);
}

void QueueEvent1(char event)
{
	events1[event]++;
}

void QueueEvent2(char event)
{
	events2[event]++;
}

char DequeueEvent1()
{
	if (events1[ON_UART1_RECEIVE] > 0)
	{
		events1[ON_UART1_RECEIVE]--;
		return ON_UART1_RECEIVE;
	}
	else if (events1[ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED] > 0)
	{
		events1[ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED]--;
		return ON_UNDIRECTED_ADVERTISEMENT_TIME_PASSED;
	}
	else
	{
		return NO_EVENT;
	}
}

char DequeueEvent2()
{
	if (events2[ON_UART2_RECEIVE] > 0)
	{
		events2[ON_UART2_RECEIVE]--;
		return ON_UART2_RECEIVE;
	}
	else
	{
		return NO_EVENT;
	}
}
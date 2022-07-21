//
// System time handlers
//
// by James Hammons
// (C) 2010 Underground Software
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  PL = PvtLewis <from Atari Age>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JPM  March/2022  Added the save state patch from PvtLewis
//

//
// STILL TO DO:
//
// - Handling for an event that occurs NOW
//

#include "event.h"

#include <string.h>
#include <stdint.h>
#include "log.h"
#include "state.h"

//#define EVENT_LIST_SIZE       512
#define EVENT_LIST_SIZE       32


// Now, a bit of weirdness: It seems that the number of lines displayed on the screen
// makes the effective refresh rate either 30 or 25 Hz!

// NOTE ABOUT TIMING SYSTEM DATA STRUCTURES:

// A queue won't work for this system because we can't guarantee that an event will go
// in with a time that is later than the ones already queued up. So we just use a simple
// list.

// Although if we used an insertion sort we could, but it wouldn't work for adjusting
// times... (For that, you would have to remove the event then reinsert it.)

struct Event
{
	bool valid;
	int eventType;
	double eventTime;
	void (* timerCallback)(void);
	uint32_t timerCallbackType;
};


static Event eventList[EVENT_LIST_SIZE];
static Event eventListJERRY[EVENT_LIST_SIZE];
static uint32_t nextEvent;
static uint32_t nextEventJERRY;
static uint32_t numberOfEvents;

extern void DSPSampleCallback(void);
extern void HalflineCallback(void);
extern void JERRYPIT1Callback(void);
extern void JERRYPIT2Callback(void);
extern void JERRYI2SCallback(void);
extern void TOMPITCallback(void);
#define TR_DSPSampleCallback 0x101
#define TR_HalflineCallback  0x201
#define TR_JERRYPIT1Callback 0x301
#define TR_JERRYPIT2Callback 0x302
#define TR_JERRYI2SCallback  0x303
#define TR_TOMPITCallback    0x401

size_t events_dump(FILE *fp)
{
	size_t total_dumped = 0;

//#define TRANSLATE_DUMP(_x) do { if (tr_eventList[i].timerCallback == _x) { tr_eventList[i].timerCallback = (void(*)())TR_##_x; goto next; } } while (0)
//#define TRANSLATE_LOAD(_x) do { if (tr_eventList[i].timerCallback == (void(*)())TR_##_x) { tr_eventList[i].timerCallback = _x; goto next; } } while (0)
//#define TRANSLATE_DUMP_JERRY(_x) do { if (tr_eventListJERRY[i].timerCallback == _x) { tr_eventListJERRY[i].timerCallback = (void(*)())TR_##_x; goto nextJERRY; } } while (0)
//#define TRANSLATE_LOAD_JERRY(_x) do { if (tr_eventListJERRY[i].timerCallback == (void(*)())TR_##_x) { tr_eventListJERRY[i].timerCallback = _x; goto nextJERRY; } } while (0)
#define TRANSLATE_DUMP(_x) do { if (tr_eventList[i].timerCallback == _x) { tr_eventList[i].timerCallbackType = TR_##_x; goto next; } } while (0)
#define TRANSLATE_LOAD(_x) do { if (tr_eventList[i].timerCallbackType == TR_##_x) { tr_eventList[i].timerCallback = _x; goto next; } } while (0)
#define TRANSLATE_DUMP_JERRY(_x) do { if (tr_eventListJERRY[i].timerCallback == _x) { tr_eventListJERRY[i].timerCallbackType = TR_##_x; goto nextJERRY; } } while (0)
#define TRANSLATE_LOAD_JERRY(_x) do { if (tr_eventListJERRY[i].timerCallbackType == TR_##_x) { tr_eventListJERRY[i].timerCallback = _x; goto nextJERRY; } } while (0)

	Event tr_eventList[EVENT_LIST_SIZE];
	Event tr_eventListJERRY[EVENT_LIST_SIZE];
	memcpy(tr_eventList, eventList, sizeof(tr_eventList));
	memcpy(tr_eventListJERRY, eventListJERRY, sizeof(tr_eventListJERRY));

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
	TRANSLATE_DUMP(DSPSampleCallback);
	TRANSLATE_DUMP(HalflineCallback);
	TRANSLATE_DUMP(JERRYPIT1Callback);
	TRANSLATE_DUMP(JERRYPIT2Callback);
	TRANSLATE_DUMP(JERRYI2SCallback);
	TRANSLATE_DUMP(TOMPITCallback);
next: {}
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		TRANSLATE_DUMP_JERRY(DSPSampleCallback);
		TRANSLATE_DUMP_JERRY(HalflineCallback);
		TRANSLATE_DUMP_JERRY(JERRYPIT1Callback);
		TRANSLATE_DUMP_JERRY(JERRYPIT2Callback);
		TRANSLATE_DUMP_JERRY(JERRYI2SCallback);
		TRANSLATE_DUMP_JERRY(TOMPITCallback);
nextJERRY: {}
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		Event *event = &tr_eventList[i];
		DUMPBOOL(event->valid);
		DUMPINT(event->eventType);
		if (event->valid)
		{
			DUMPDOUBLE(event->eventTime);
		}
		//uint32_t callback = ((unsigned long)event->timerCallback) & 0xffffffff;
		//DUMP32(callback);
		DUMP32(event->timerCallbackType);
		//WriteLog("event dumped %d %d %f %p %u\n", event->valid, event->eventType, event->eventTime, event->timerCallback, event->timerCallbackType);
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		Event *event = &tr_eventListJERRY[i];
		DUMPBOOL(event->valid);
		DUMPINT(event->eventType);
		if (event->valid)
		{
			DUMPDOUBLE(event->eventTime);
		}
		//uint32_t callback = ((unsigned long)event->timerCallback) & 0xffffffff;
		//DUMP32(callback);
		DUMP32(event->timerCallbackType);
		//WriteLog("event dumped %d %d %f %p %u\n", event->valid, event->eventType, event->eventTime, event->timerCallback, event->timerCallbackType);
	}

	DUMP32(nextEvent);
	DUMP32(nextEventJERRY);
	DUMP32(numberOfEvents);

	return total_dumped;
}

size_t events_load(FILE *fp)
{
	size_t total_loaded = 0;

	Event tr_eventList[EVENT_LIST_SIZE];
	Event tr_eventListJERRY[EVENT_LIST_SIZE];

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		Event *event = &tr_eventList[i];
		LOADBOOL(event->valid);
		LOADINT(event->eventType);
		if (event->valid)
		{
			LOADDOUBLE(event->eventTime);
		}
		//uint32_t callback;
		//LOAD32(callback);
		//event->timerCallback = (void(*)())callback;
		LOAD32(event->timerCallbackType);
		event->timerCallback = NULL;
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		Event *event = &tr_eventListJERRY[i];
		LOADBOOL(event->valid);
		LOADINT(event->eventType);
		if (event->valid)
		{
			LOADDOUBLE(event->eventTime);
		}
		//uint32_t callback;
		//LOAD32(callback);
		//event->timerCallback = (void(*)())callback;
		LOAD32(event->timerCallbackType);
		event->timerCallback = NULL;
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		TRANSLATE_LOAD(DSPSampleCallback);
		TRANSLATE_LOAD(HalflineCallback);
		TRANSLATE_LOAD(JERRYPIT1Callback);
		TRANSLATE_LOAD(JERRYPIT2Callback);
		TRANSLATE_LOAD(JERRYI2SCallback);
		TRANSLATE_LOAD(TOMPITCallback);
next: {}
	}

	for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++)
	{
		TRANSLATE_LOAD_JERRY(DSPSampleCallback);
		TRANSLATE_LOAD_JERRY(HalflineCallback);
		TRANSLATE_LOAD_JERRY(JERRYPIT1Callback);
		TRANSLATE_LOAD_JERRY(JERRYPIT2Callback);
		TRANSLATE_LOAD_JERRY(JERRYI2SCallback);
		TRANSLATE_LOAD_JERRY(TOMPITCallback);
nextJERRY: {}
	}

	//for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++) {
	//  Event *event = &tr_eventList[i];
	//  WriteLog("event loaded %d %d %f %p %u\n", event->valid, event->eventType, event->eventTime, event->timerCallback, event->timerCallbackType);
	//}
	//for (uint32_t i = 0; i < EVENT_LIST_SIZE; i++) {
	//  Event *event = &tr_eventListJERRY[i];
	//  WriteLog("event loaded %d %d %f %p %u\n", event->valid, event->eventType, event->eventTime, event->timerCallback, event->timerCallbackType);
	//}

	memcpy(eventList, tr_eventList, sizeof(eventList));
	memcpy(eventListJERRY, tr_eventListJERRY, sizeof(eventListJERRY));
	LOAD32(nextEvent);
	LOAD32(nextEventJERRY);
	LOAD32(numberOfEvents);

	return total_loaded;
}

void EventsReload(void)
{
	WriteLog("numberOfEvents: %u\n", (unsigned int)(numberOfEvents & 0xffffff));
}


void InitializeEventList(void)
{
	for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
	{
		eventList[i].valid = false;
		eventListJERRY[i].valid = false;
	}

	numberOfEvents = 0;
	WriteLog("EVENT: Cleared event list.\n");
}


// Set callback time in µs. This is fairly arbitrary, but works well enough for our purposes.
//We just slap the next event into the list in the first available slot, no checking, no nada...
void SetCallbackTime(void (* callback)(void), double time, int type/*= EVENT_MAIN*/)
{
	if (type == EVENT_MAIN)
	{
		for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
		{
			if (!eventList[i].valid)
			{
//WriteLog("EVENT: Found callback slot #%u...\n", i);
				eventList[i].timerCallback = callback;
				eventList[i].eventTime = time;
				eventList[i].eventType = type;
				eventList[i].valid = true;
				numberOfEvents++;

				return;
			}
		}

		WriteLog("EVENT: SetCallbackTime() failed to find an empty slot in the main list (%u events)!\n", numberOfEvents);
	}
	else
	{
		for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
		{
			if (!eventListJERRY[i].valid)
			{
//WriteLog("EVENT: Found callback slot #%u...\n", i);
				eventListJERRY[i].timerCallback = callback;
				eventListJERRY[i].eventTime = time;
				eventListJERRY[i].eventType = type;
				eventListJERRY[i].valid = true;
				numberOfEvents++;

				return;
			}
		}

		WriteLog("EVENT: SetCallbackTime() failed to find an empty slot in the main list (%u events)!\n", numberOfEvents);
	}
}


void RemoveCallback(void (* callback)(void))
{
	for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
	{
		if (eventList[i].valid && eventList[i].timerCallback == callback)
		{
			eventList[i].valid = false;
			numberOfEvents--;

			return;
		}
		else if (eventListJERRY[i].valid && eventListJERRY[i].timerCallback == callback)
		{
			eventListJERRY[i].valid = false;
			numberOfEvents--;

			return;
		}
	}
}


void AdjustCallbackTime(void (* callback)(void), double time)
{
	for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
	{
		if (eventList[i].valid && eventList[i].timerCallback == callback)
		{
			eventList[i].eventTime = time;

			return;
		}
		else if (eventListJERRY[i].valid && eventListJERRY[i].timerCallback == callback)
		{
			eventListJERRY[i].eventTime = time;

			return;
		}
	}
}


//
// Since our list is unordered WRT time, we have to search it to find the next event
// Returns time to next event & sets nextEvent to that event
//
double GetTimeToNextEvent(int type/*= EVENT_MAIN*/)
{
#if 0
	double time = 0;
	bool firstTime = true;

	for(uint32 i=0; i<EVENT_LIST_SIZE; i++)
	{
		if (eventList[i].valid)
		{
			if (firstTime)
				time = eventList[i].eventTime, nextEvent = i, firstTime = false;
			else
			{
				if (eventList[i].eventTime < time)
					time = eventList[i].eventTime, nextEvent = i;
			}
		}
	}
#else
	if (type == EVENT_MAIN)
	{
		double time = eventList[0].eventTime;
		nextEvent = 0;

		for(uint32_t i=1; i<EVENT_LIST_SIZE; i++)
		{
			if (eventList[i].valid && (eventList[i].eventTime < time))
			{
				time = eventList[i].eventTime;
				nextEvent = i;
			}
		}

		return time;
	}
	else
	{
		double time = eventListJERRY[0].eventTime;
		nextEventJERRY = 0;

		for(uint32_t i=1; i<EVENT_LIST_SIZE; i++)
		{
			if (eventListJERRY[i].valid && (eventListJERRY[i].eventTime < time))
			{
				time = eventListJERRY[i].eventTime;
				nextEventJERRY = i;
			}
		}

		return time;
	}
#endif
}


void HandleNextEvent(int type/*= EVENT_MAIN*/)
{
	if (type == EVENT_MAIN)
	{
		double elapsedTime = eventList[nextEvent].eventTime;
		void (* event)(void) = eventList[nextEvent].timerCallback;

		for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
		{
	//We can skip the check & just subtract from everything, since the check is probably
	//just as heavy as the code after and we won't use the elapsed time from an invalid event anyway.
	//		if (eventList[i].valid)
				eventList[i].eventTime -= elapsedTime;
		}

		eventList[nextEvent].valid = false;			// Remove event from list...
		numberOfEvents--;

		(*event)();
	}
	else
	{
		double elapsedTime = eventListJERRY[nextEventJERRY].eventTime;
		void (* event)(void) = eventListJERRY[nextEventJERRY].timerCallback;

		for(uint32_t i=0; i<EVENT_LIST_SIZE; i++)
		{
	//We can skip the check & just subtract from everything, since the check is probably
	//just as heavy as the code after and we won't use the elapsed time from an invalid event anyway.
	//		if (eventList[i].valid)
				eventListJERRY[i].eventTime -= elapsedTime;
		}

		eventListJERRY[nextEventJERRY].valid = false;	// Remove event from list...
		numberOfEvents--;

		(*event)();
	}
}


/*
void OPCallback(void)
{
	DoFunkyOPStuffHere();

	SetCallbackTime(OPCallback, HORIZ_PERIOD_IN_USEC);
}

void VICallback(void)
{
	double oneFrameInUsec = 16666.66666666;
	SetCallbackTime(VICallback, oneFrameInUsec / numberOfLines);
}

void JaguarInit(void)
{
	double oneFrameInUsec = 16666.66666666;
	SetCallbackTime(VICallback, oneFrameInUsec / numberOfLines);
	SetCallbackTime(OPCallback, );
}

void JaguarExec(void)
{
	while (true)
	{
		double timeToNextEvent = GetTimeToNextEvent();

		m68k_execute(USEC_TO_M68K_CYCLES(timeToNextEvent));
		GPUExec(USEC_TO_RISC_CYCLES(timeToNextEvent));
		DSPExec(USEC_TO_RISC_CYCLES(timeToNextEvent));

		if (!HandleNextEvent())
			break;
	}
}

// NOTES: The timers count RISC cycles, and when the dividers count down to zero they can interrupt either the DSP and/or CPU.

// NEW:
// TOM Programmable Interrupt Timer handler
// NOTE: TOM's PIT is only enabled if the prescaler is != 0
//       The PIT only generates an interrupt when it counts down to zero, not when loaded!

void TOMResetPIT()
{
	// Need to remove previous timer from the queue, if it exists...
	RemoveCallback(TOMPITCallback);

	if (TOMPITPrescaler)
	{
		double usecs = (TOMPITPrescaler + 1) * (TOMPITDivider + 1) * RISC_CYCLE_IN_USEC;
		SetCallbackTime(TOMPITCallback, usecs);
	}
}

void TOMPITCallback(void)
{
	INT1_RREG |= 0x08;                         // Set TOM PIT interrupt pending
	GPUSetIRQLine(GPUIRQ_TIMER, ASSERT_LINE);  // It does the 'IRQ enabled' checking

	if (INT1_WREG & 0x08)
		m68k_set_irq(2);                       // Generate 68K NMI

	TOMResetPIT();
}

// Small problem with this approach: If a timer interrupt is already pending,
// the pending timer needs to be replaced with the new one! (Taken care of above, BTW...)

TOMWriteWord(uint32 address, uint16 data)
{
	if (address == PIT0)
	{
		TOMPITPrescaler = data;
		TOMResetPIT();
	}
	else if (address == PIT1)
	{
		TOMPITDivider = data;
		TOMResetPIT();
	}
}

*/

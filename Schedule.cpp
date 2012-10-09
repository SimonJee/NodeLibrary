/**
  \file Schedule.cpp 
  \brief Scheduling
  \author SimonJee
  \date Created on October 2, 2012.
  \copyright Released into the public domain.
*/

#include "Arduino.h"

#include "Schedule.h"
#include <Ports.h>

/** \brief Internally used only. \internal Masks the schedule type */
#define SCHEDULE_TYPE_MASK				(3)

/** \brief Internally used only. \internal Free slot */
#define SCHEDULE_TYPE_NONE				0

/** \brief Internally used only. \internal Execute once, parameters in milliseconds */
#define SCHEDULE_TYPE_MS_ONCE			1

/** \brief Internally used only. \internal Execute periodic, parameters in milliseconds */
#define SCHEDULE_TYPE_MS_PERIODIC 		2

// TODO: Add 3,4 for other unit?

/** \brief Internally used only. \internal Masks the schedule entry priority */
#define SCHEDULE_PRIORITY_MASK			(0x18)

/** \brief Internally used only. \internal Try to run on schedule even if the last execution was delayed. */
#define SCHEDULE_HIGH_ACCURACY			32

/** \brief Internally used only. \internal Marker bit event handler requires handler parameter. */
#define SCHEDULE_CALLBACK_WITH_HANDLE 	64

#define HANDLE_TO_INDEX(handle)			(handle - 1)
#define INDEX_TO_HANDLE(index)			(index + 1)

#define _UNDERFLOW_MARK					60000

/**
 * \brief Interrupt handler for watchdog events.
 * 
 * Calls Sleepy::watchdogEvent() from the JeeLib library. This interrrupt handler is neccessary to recover from deep sleep.
 */
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

volatile Schedule::ScheduleEntry Schedule::entries[MAX_SCHEDULE_ENTRIES] = {};

void Schedule::loopAndSleep() {
	word msToNextTask = loop();

	// TODO: Replace? interrupt support not very good, no rtc sync
	Sleepy::loseSomeTime(msToNextTask > _UNDERFLOW_MARK ? _UNDERFLOW_MARK : msToNextTask); 
}

word Schedule::loop() {
	// Use offset to give each task the same chance of running
	static byte offset = 0;
	offset++;

	byte run_normal = 0;
	word run_normal_remaining = ~0;
	byte run_bg = 0;
	word run_bg_remaining = ~0;

	word nextScheduled = _UNDERFLOW_MARK;
	byte remaining = ~0;

	// loop over all schedule slots, execute all priority tasks directly	
	for(uint8_t i = offset; i != (MAX_SCHEDULE_ENTRIES + offset); ++i) {
		byte index = i % MAX_SCHEDULE_ENTRIES;

		if(entries[index].scheduleType != SCHEDULE_TYPE_NONE)  {
			volatile ScheduleEntry* curr = &entries[index];
			remaining = curr->next - millis();

			// Underflow we should run now
			if(remaining > _UNDERFLOW_MARK) {
				byte prio = (curr->scheduleType & SCHEDULE_PRIORITY_MASK);
				if(prio == SCHEDULE_PRIORITY_HIGH)
				{
					remaining = executeEntry(index);
				}
				else if (prio == SCHEDULE_PRIORITY_BACKGROUND) 
				{
					if(remaining < run_bg_remaining) {
						run_bg_remaining = remaining;
						run_bg = index;
					}
				}
				else {
					if(remaining < run_normal_remaining) {
						run_normal_remaining = remaining;
						run_normal = index;
					}
				}
			}
			
			if(remaining < nextScheduled) {
				nextScheduled = remaining;	
			}			
		}

		if(run_normal_remaining != (word)~0) {
			remaining = executeEntry(run_normal);
		}
		else if(run_bg_remaining != (word)~0) {
			remaining = executeEntry(run_bg);
		}

		if(remaining < nextScheduled) {
			nextScheduled = remaining;	
		}		
	}

	return nextScheduled;
}

word Schedule::executeEntry(byte index) {
	volatile ScheduleEntry* entry = &entries[index];

	if(entry->scheduleType & SCHEDULE_CALLBACK_WITH_HANDLE) {
		((void (*)(byte))entry->scheduleEvent)(INDEX_TO_HANDLE(index));
	} else {
		entry->scheduleEvent();
	}

	switch(entry->scheduleType & SCHEDULE_TYPE_MASK) {
		case SCHEDULE_TYPE_MS_PERIODIC:					
			if(entry->scheduleType != 0) {
				if(entry->scheduleType & SCHEDULE_HIGH_ACCURACY) {
					// Do not lose the time needed waiting for execution
					entry->next = entry->next + entry->period - 1;
				} else {
					// Lose the time needed for waiting and execution...
					entry->next = millis() + entry->period - 1;
				}
				return entry->next - millis(); // Schedule changed
			}
			break; // return 0; do not execute again
		case SCHEDULE_TYPE_MS_ONCE:
			entry->scheduleType = 0;						
			break; // return 0; do not execute again
	}

	return (word)~0;
}

void Schedule::remove(byte handle) {
	byte i = HANDLE_TO_INDEX(handle);
	entries[i].scheduleType = 0;
	//entries[i].next = 0;
	//entries[i].period = 0;
	//entries[i].scheduleEvent = NULL;	
}

byte Schedule::findFreeHandle() {
	for(byte i = 0; i <= MAX_SCHEDULE_ENTRIES; i++) {		
		if(entries[i].scheduleType == 0) {
			return INDEX_TO_HANDLE(i);			
		}
	}
	
	return 0;
}

byte Schedule::run(void (*callback)(), byte priority) {
	return Schedule::after(0, callback, priority);
}

byte Schedule::run(void (*callback)(byte), byte priority) {
	return Schedule::after(0, callback, priority);
}

byte Schedule::after(word ms, void (*callback)(), byte priority) {
	byte handle = findFreeHandle();
	
	if(handle) {
		byte i = HANDLE_TO_INDEX(handle);
		entries[i].scheduleType = SCHEDULE_TYPE_MS_ONCE;		
		entries[i].next = millis() + ms - 1;
		entries[i].scheduleEvent = callback;		
	}
	
	return handle;
}

byte Schedule::after(word ms, void (*callback)(byte), byte priority) {
	byte handle = after(ms, (void (*)())callback, priority);
	
	if(handle) {
		entries[HANDLE_TO_INDEX(handle)].scheduleType |= SCHEDULE_CALLBACK_WITH_HANDLE;		
	}
	
	return handle;
}

byte Schedule::every(word ms, void (*callback)(), byte priority, word startDelay) {
	byte handle = findFreeHandle();
	
	if(handle) {
		byte i = HANDLE_TO_INDEX(handle);

		entries[i].scheduleType = SCHEDULE_TYPE_MS_PERIODIC;
		entries[i].period = ms;
		
		if(startDelay == (word)-1) {
			entries[i].next = millis() + ms - 1;
		} else {
			entries[i].next = millis() + startDelay - 1;
		}
		
		entries[i].scheduleEvent = callback;		
	}
	
	return handle;
}

byte Schedule::every(word ms, void (*callback)(byte), byte priority, word startDelay) {
	byte handle = every(ms, (void (*)())callback, priority, startDelay);
	
	if(handle) {
		entries[HANDLE_TO_INDEX(handle)].scheduleType |= SCHEDULE_CALLBACK_WITH_HANDLE;		
	}
	
	return handle;
}
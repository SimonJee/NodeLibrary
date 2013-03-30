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

//TODO: Remove me
#include <avr/sleep.h>
#include <util/atomic.h>

/** \brief Internally used only. \internal Masks the schedule unit */

#define SCHEDULE_UNIT_MASK				(3)

#define SCHEDULE_NONE					0
#define SCHEDULE_UNIT_MS				1
#define SCHEDULE_UNIT_S					2
#define SCHEDULE_UNIT_M					3

/*#define SCHEDULE_REPEAT_MASK			(4)

#define SCHEDULE_REPEAT					0
#define SCHEDULE_NO_REPEAT				4*/


/** \brief Internally used only. \internal Masks the schedule entry priority */
#define SCHEDULE_PRIORITY_MASK			(0x30)

/** \brief Internally used only. \internal Marker bit event handler requires handler parameter. */
#define SCHEDULE_CALLBACK_WITH_HANDLE 	128

#define HANDLE_TO_INDEX(handle)			(handle - 1)
#define INDEX_TO_HANDLE(index)			(index + 1)

#define _UNDERFLOW_MARK					60000

/**
* \brief Interrupt handler for watchdog events.
* 
* Calls Sleepy::watchdogEvent() from the JeeLib library. This interrrupt handler is neccessary to recover from deep sleep.
*/
ISR(WDT_vect) { Sleepy::watchdogEvent(); }


ScheduleI::ScheduleEntry ScheduleI::entries[MAX_SCHEDULE_ENTRIES] = {};
/*volatile byte ScheduleI::handleInterrupt = 0;
byte ScheduleI::sleepMode = SLEEP_MODE_IDLE;*/

// Todo: FIxme
//Port led(1);

ScheduleI::ScheduleI() : sleepMode(SLEEP_MODE_IDLE) {}

void ScheduleI::loopAndSleep() {
	//led.mode(OUTPUT);
	//led.digiWrite(1);
	uint16_t msToNextTask = loop();

	if(msToNextTask > 1)
	{		
		if(sleepMode == SLEEP_MODE_IDLE) {
			//led.digiWrite(0);

			// We will wake up from software timer approx. every ms

			set_sleep_mode(SLEEP_MODE_IDLE);
			msToNextTask = millis() + msToNextTask - 1;

			uint16_t remaining = 0; 

			do {				
				cli();
				if (!handleInterrupt)
				{
					sleep_enable();
					sei();
					sleep_cpu();
					sleep_disable();

					remaining = msToNextTask - millis();
				}
				sei();				
			} while(remaining < _UNDERFLOW_MARK  && !handleInterrupt);
		}
		else if(sleepMode == SLEEP_MODE_PWR_DOWN) {
			//led.digiWrite(0);

			// TODO: Replace... Might have a problem with big numbers
			if(!Sleepy::loseSomeTime(msToNextTask)){
				ScheduleI::interrupt();			
			}
		}
	}
}

void ScheduleI::powerDown() {
	//led.digiWrite(0);

	// From JeeLib:

	byte adcsraSave = ADCSRA;
	ADCSRA &= ~ bit(ADEN); // disable the ADC
	// see http://www.nongnu.org/avr-libc/user-manual/group__avr__sleep.html
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		sleep_enable();
		// sleep_bod_disable(); // can't use this - not in my avr-libc version!
#ifdef BODSE
		MCUCR = MCUCR | bit(BODSE) | bit(BODS); // timed sequence
		MCUCR = MCUCR & ~ bit(BODSE) | bit(BODS);
#endif
	}
	sleep_cpu();
	sleep_disable();
	// re-enable what we disabled
	ADCSRA = adcsraSave;

	// JeeLib end

	ScheduleI::interrupt();
}

uint16_t ScheduleI::loop() {	
	static uint8_t offset = 0;

	byte run_normal = 0;
	uint16_t run_normal_remaining = (uint16_t)~0;
	byte run_bg = 0;
	uint16_t run_bg_remaining = (uint16_t)~0;

	uint16_t nextScheduled = (uint16_t)~0;
	uint16_t remaining = (uint16_t)~0;

	offset++;
	offset = offset % (((byte)~0) - MAX_SCHEDULE_ENTRIES);

	if(handleInterrupt) {
		for(uint8_t i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
			if((entries[i].scheduleType & SCHEDULE_PRIORITY_MASK) == SCHEDULE_PRIORITY_INTERRUPT && entries[i].period != (uint16_t)~0)  {
				Serial.print("\r\nMarked interrupt for execution\r\n");
				Serial.flush();
				ScheduleI::update(INDEX_TO_HANDLE(i), entries[i].period, 0);
			}
		}

		handleInterrupt = 0;
	}

	// loop over all schedule slots, execute all priority tasks directly	
	for(uint8_t i = offset; i < (offset + MAX_SCHEDULE_ENTRIES); i++) {
		byte index = i % MAX_SCHEDULE_ENTRIES;

		if((entries[index].scheduleType & SCHEDULE_UNIT_MASK) != SCHEDULE_NONE)  {
			volatile ScheduleEntry* curr = &(entries[index]);
			remaining = curr->next - millis();
			byte prio = (curr->scheduleType & SCHEDULE_PRIORITY_MASK);

			// Underflow we should run now
			if(remaining >= _UNDERFLOW_MARK) {
				// Remaining might be the special value ~0 so decrement by one to make further use easier
				remaining--;				

				if(prio == SCHEDULE_PRIORITY_HIGH || prio == SCHEDULE_PRIORITY_INTERRUPT)
				{
					remaining = executeEntry(index);
				}
				else if (prio == SCHEDULE_PRIORITY_BACKGROUND) 
				{
					// Do we have already something to do?
					if(run_bg_remaining != (uint16_t)~0) {
						nextScheduled = 0;
					}

					if(remaining < run_bg_remaining) {						
						run_bg_remaining = remaining;
						run_bg = index;
					}
				}
				else {
					// Do we have already something to do?
					if(run_normal_remaining != (uint16_t)~0) {
						nextScheduled = 0;
					}

					if(remaining < run_normal_remaining) {						
						run_normal_remaining = remaining;
						run_normal = index;
					}
				}
			}

			if(prio != SCHEDULE_PRIORITY_BACKGROUND && remaining < nextScheduled) {
				nextScheduled = remaining;	
			}			
		}
	}

	if(run_normal_remaining != (uint16_t)~0) {
		remaining = executeEntry(run_normal);
	}
	else if(run_bg_remaining != (uint16_t)~0) {
		remaining = executeEntry(run_bg);
	}	

	if(remaining < nextScheduled) {
		nextScheduled = remaining;	
	}	

	return nextScheduled;
}

uint16_t ScheduleI::executeEntry(byte index) {
	volatile ScheduleEntry* entry = &entries[index];

	if(entry->scheduleType & SCHEDULE_CALLBACK_WITH_HANDLE) {
		((void (*)(byte))entry->scheduleEvent)(INDEX_TO_HANDLE(index));
	} else {
		entry->scheduleEvent();
	}

	if((entry->scheduleType & SCHEDULE_UNIT_MASK) > 0) {
		if(entry->period != (uint16_t)~0) {
			entry->next = millis() + entry->period - 1;			
			return entry->period; // Schedule changed
		}
		else
		{
			entry->scheduleType = SCHEDULE_NONE;						
		}
	}

	return (uint16_t)~0;
}

void ScheduleI::interrupt() {
	handleInterrupt = 1;
}

void ScheduleI::setSleepMode(byte mode) {
	sleepMode = mode;
}

void ScheduleI::remove(byte handle) {
	if(!handle) return;
	byte i = HANDLE_TO_INDEX(handle);
	entries[i].scheduleType = 0;
}

byte ScheduleI::findFreeHandle() {
	for(byte i = 0; i <= MAX_SCHEDULE_ENTRIES; i++) {		
		if(entries[i].scheduleType == 0) {
			return INDEX_TO_HANDLE(i);			
		}
	}

	return 0;
}

byte ScheduleI::run(void (*callback)(), byte options) {
	return ScheduleI::after(0, callback, options);
}

byte ScheduleI::run(void (*callback)(byte), byte options) {
	return ScheduleI::after(0, callback, options);
}

byte ScheduleI::after(uint16_t ms, void (*callback)(), byte options) {
	return every((uint16_t)~0, callback, options, ms);	
}

byte ScheduleI::after(uint16_t ms, void (*callback)(byte), byte options) {
	return every((uint16_t)~0, callback, options, ms);
}


byte ScheduleI::update(byte handle, uint16_t ms, uint16_t startDelay) {
	if(handle) {
		byte i = HANDLE_TO_INDEX(handle);

		if((entries[i].scheduleType & SCHEDULE_UNIT_MASK) == 0) {
			return 0;
		}

		// Do not allow to change from "after"-schedule-type to "every"
		if(entries[i].period != (uint16_t)~0) {
			entries[i].period = ms;
		}

		if(startDelay == (uint16_t)~0) {
			entries[i].next = millis() + ms - 1;
		} else {
			entries[i].next = millis() + startDelay - 1;
		}		
	}

	return handle;
}

byte ScheduleI::every(uint16_t ms, void (*callback)(), byte options, uint16_t startDelay) {
	byte handle = findFreeHandle();

	if(handle) {
		byte i = HANDLE_TO_INDEX(handle);

		entries[i].scheduleType = SCHEDULE_UNIT_MS | (options & SCHEDULE_PRIORITY_MASK);
		entries[i].period = ms;

		if(startDelay == (uint16_t)~0) {
			entries[i].next = millis() + ms - 1;
		} else {
			entries[i].next = millis() + startDelay - 1;
		}

		entries[i].scheduleEvent = callback;		
	}

	return handle;
}


byte ScheduleI::every(uint16_t ms, void (*callback)(byte), byte options, uint16_t startDelay) {
	byte handle = every(ms, (void (*)())callback, options, startDelay);

	if(handle) {
		entries[HANDLE_TO_INDEX(handle)].scheduleType |= SCHEDULE_CALLBACK_WITH_HANDLE;		
	}

	return handle;
}

ScheduleI Schedule;
/*
  Schedule.cpp - Library for communicating with wireless nodes.  
  Created by SimonJee, October 2, 2012.
  Released into the public domain.
*/

#include "Arduino.h"

#include "Schedule.h"
#include <Ports.h>

// Enable watchdog interrupt
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

Schedule::ScheduleEntry Schedule::entries[MAX_SCHEDULE_ENTRIES];

void Schedule::loopAndSleep() {
	word next2 = 0;
	word remain = 0;
	
	for(byte i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {		
		if(entries[i].scheduleType != 0 && entries[i].scheduleEvent != NULL) {
			remain = entries[i].next - millis();
			
			// Overflow means timeout reached, see JeeLib MilliTimer
			if (remain > 60000) {		
				if(entries[i].scheduleType & SCHEDULE_CALLBACK_WITH_HANDLE) {
					((void (*)(byte))entries[i].scheduleEvent)(i);
				} else {
					entries[i].scheduleEvent();
				}
				
				switch(entries[i].scheduleType & 0xF) {
					case SCHEDULE_TYPE_MS_PERIODIC:						
						entries[i].next = entries[i].next + entries[i].period -1;
						break;
					case SCHEDULE_TYPE_MS_ONCE:
						remove(i);
						break;
				}
			} else {
				if(next2 == 0 || next2 > remain) {
					next2 = remain;
				}
			}
		}
	}
	
	remain = next2 - millis();

	if (remain <= 60000) {
		Sleepy::loseSomeTime(16); // TODO: Does not support interrupts
	}
}

void Schedule::remove(byte handle) {
	entries[handle].scheduleType = 0;
	entries[handle].next = 0;
	entries[handle].period = 0;
	entries[handle].scheduleEvent = NULL;	
}

byte Schedule::findFreeHandle() {
	byte handle = 0;
	
	for(byte i = 0; i < MAX_SCHEDULE_ENTRIES && handle == 0; i++) {		
		if(entries[i].scheduleType == 0) {
			handle = i;			
		}
	}
	
	return handle;
}

byte Schedule::after(word ms, void (*callback)()) {
	byte handle = findFreeHandle();
	
	if(handle) {
		entries[handle].scheduleType = SCHEDULE_TYPE_MS_ONCE;
		entries[handle].period = ms;
		entries[handle].next = millis() + ms - 1;
		entries[handle].scheduleEvent = callback;		
	}
	
	return handle;
}

byte Schedule::after(word ms, void (*callback)(byte)) {
	byte handle = after(ms, (void (*)())callback);
	
	if(handle) {
		entries[handle].scheduleType |= SCHEDULE_CALLBACK_WITH_HANDLE;		
	}
	
	return handle;
}

byte Schedule::every(word ms, void (*callback)(), word startDelay) {
	byte handle = findFreeHandle();
	
	if(handle) {
		entries[handle].scheduleType = SCHEDULE_TYPE_MS_PERIODIC;
		entries[handle].period = ms;
		
		if(startDelay == -1) {
			entries[handle].next = millis() + ms - 1;
		} else {
			entries[handle].next = millis() + startDelay - 1;
		}
		
		entries[handle].scheduleEvent = callback;		
	}
	
	return handle;
}

byte Schedule::every(word ms, void (*callback)(byte), word startDelay) {
	byte handle = every(ms, (void (*)())callback, startDelay);
	
	if(handle) {
		entries[handle].scheduleType |= SCHEDULE_CALLBACK_WITH_HANDLE;		
	}
	
	return handle;
}
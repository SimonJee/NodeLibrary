/**
  \file Schedule.h 
  \brief Scheduling
  \author SimonJee
  \date Created on October 2, 2012.
  \copyright Released into the public domain.
*/

#ifndef Schedule_h
#define Schedule_h

#ifndef MAX_SCHEDULE_ENTRIES 
/**
 * \brief Defines the maximal count of concurrent scheduling entries. 
 *
 * You can override this by using \code #define MAX_SCHEDULE_ENTRIES 20\endcode before including the Schedule.h file for the first time.
 */
#define MAX_SCHEDULE_ENTRIES 16
#endif

/** \brief Run with normal priority. */
#define SCHEDULE_PRIORITY_NORMAL		0

/** \brief Run only if nothing else todo. */
#define SCHEDULE_PRIORITY_BACKGROUND	16

/** \brief Run with priority. Use with caution. */
#define SCHEDULE_PRIORITY_HIGH			32

#define SCHEDULE_PRIORITY_INTERRUPT		48


/**
 * \brief Static class for handling scheduled tasks.
 *
 * The Schedule class can be used to schedule events based on time. 
 */
class ScheduleI {
public:
	ScheduleI();

	/**
	 * \brief Evaluates the schedule plan
	 * \return The number of milliseconds until the next event.
	 */
	  uint16_t loop();
	 
	/**
	 * \brief Evaluates the schedule plan and goes to sleep mode
	 */
	 void loopAndSleep();
	
		
	/**
	 * \brief Register a callback that is executed as soon as possible
	 *	 
	 * \param callback Callback of type void myCallbackFunction()	 
	 * \param options Set the schedule options.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte run(void (*callback)(), byte options = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed as soon as possible
	 *
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param options Set the schedule options.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte run(void (*callback)(byte), byte options = SCHEDULE_PRIORITY_NORMAL);
	

	/**
	 * \brief Register a callback that is executed after the given time in ms
	 *	 
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction()	 
	 * \param options Set the schedule options.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte after(uint16_t ms, void (*callback)(), byte options = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed after the given time in ms
	 *
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param options Set the schedule options.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte after(uint16_t ms, void (*callback)(byte), byte options = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction()
	 * \param options Set the schedule options.	 
	 * \param startDelay Delay in milliseconds
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte every(uint16_t ms, void (*callback)(), byte options = SCHEDULE_PRIORITY_NORMAL, uint16_t startDelay = ~0);
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param options Set the schedule options.	
	 * \param startDelay Delay in milliseconds
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	 byte every(uint16_t ms, void (*callback)(byte), byte options = SCHEDULE_PRIORITY_NORMAL, uint16_t startDelay = ~0);
	
	
	/**
	 * \brief Updates an schedule entry
	 */
	 byte update(byte handle, uint16_t ms, uint16_t startDelay = ~0);

	/**
	 * \brief Removes the event with the specified handle from schedule
	 */
	 void remove(byte handle);

	/**
	 * \brief signals that an interrupt occured and all interrupt handler should be rescheduled to run immediately
	 */
	 void interrupt();

	/**
	 * \brief Powers down the cpu to save energy. The shedule is halted at the current point and millis(9 are not updated any longer
	 */
	 void powerDown();

	// TODO: allow to set the correct sleep mode for idle (power down might be a bad choice if a pwm is used)
	 void setSleepMode(byte mode);
private:
	/**
	 * \brief Struct for schedule entries
	 */
	struct __attribute__((packed)) ScheduleEntry {		
		uint16_t period;
		uint16_t next;
		byte scheduleType;
		void (*scheduleEvent)();
	};
	
	/**
	 * \brief Storage for the tasks
	 */
	 static ScheduleEntry entries[MAX_SCHEDULE_ENTRIES];
	
	/**
	 * \brief Returns the next free handle; 0 otherwise
	 */
	 byte findFreeHandle();

	/**
	 * \brief Executes the entry by the given index. Do not call directly.
	 *
	 * \param index The index of the entry to execute
	 *
	 * \return count of milliseconds to next run if the entry úpdated its schedule to run again; ~0 otherwise.
	 */
	 uint16_t executeEntry(byte index);

	/**
	 * \brief Whether an interrupt occured
	 */
	 volatile byte handleInterrupt;

	
	 byte sleepMode;
};


extern ScheduleI Schedule;

#endif
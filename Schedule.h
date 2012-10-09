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
#define MAX_SCHEDULE_ENTRIES 10
#endif

/** \brief Run with normal priority. */
#define SCHEDULE_PRIORITY_NORMAL		0

/** \brief Run only if nothing else todo. */
#define SCHEDULE_PRIORITY_BACKGROUND	8

/** \brief Run with priority. Use with caution. */
#define SCHEDULE_PRIORITY_HIGH			16


/**
 * \brief Static class for handling scheduled tasks.
 *
 * The Schedule class can be used to schedule events based on time. 
 */
class Schedule {
public:
	/**
	 * \brief Evaluates the schedule plan
	 * \return The number of milliseconds until the next event.
	 */
	 static word loop();
	 
	/**
	 * \brief Evaluates the schedule plan and goes to sleep mode
	 */
	static void loopAndSleep();
	
		
	/**
	 * \brief Register a callback that is executed as soon as possible
	 *	 
	 * \param callback Callback of type void myCallbackFunction()	 
	 * \param priority Set the schedule priority.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte run(void (*callback)(), byte priority = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed as soon as possible
	 *
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param priority Set the schedule priority.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte run(void (*callback)(byte), byte priority = SCHEDULE_PRIORITY_NORMAL);
	

	/**
	 * \brief Register a callback that is executed after the given time in ms
	 *	 
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction()	 
	 * \param priority Set the schedule priority.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte after(word ms, void (*callback)(), byte priority = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed after the given time in ms
	 *
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param priority Set the schedule priority.
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte after(word ms, void (*callback)(byte), byte priority = SCHEDULE_PRIORITY_NORMAL);
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction()
	 * \param priority Set the schedule priority.	 
	 * \param startDelay Delay in milliseconds
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte every(word ms, void (*callback)(), byte priority = SCHEDULE_PRIORITY_NORMAL, word startDelay = -1);
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param priority Set the schedule priority.	
	 * \param startDelay Delay in milliseconds
	 *
	 * \return a handle to the task entry on success; 0 otherwise
	 */
	static byte every(word ms, void (*callback)(byte), byte priority = SCHEDULE_PRIORITY_NORMAL, word startDelay = -1);
	
	/**
	 * \brief Registers or updates a timeout with the given handle.
	 *
	 * The timeout is called after the given time has passed without updating the timeout.	 
	 * If you call update on a handle that is currently not registered 0 is returned. You should reset your handle in the timeout function to 0.
	 *
	 * \param handle Handle of the timeout
	 * \return a handle to the task entry on success; 0 otherwise.
	 */
	//TODO
	//static byte timeout(byte handle, word ms, void (*callback)(byte), byte priority = SCHEDULE_PRIORITY_NORMAL);

	/**
	 * \brief Removes the event with the specified handle from schedule
	 */
	static void remove(byte handle);
private:
	/**
	 * \brief Struct for schedule entries
	 */
	struct __attribute__((packed)) ScheduleEntry {		
		word period;
		word next;
		byte scheduleType;
		void (*scheduleEvent)();
	};
	
	/**
	 * \brief Storage for the tasks
	 */
	static volatile ScheduleEntry entries[MAX_SCHEDULE_ENTRIES];
	
	/**
	 * \brief Returns the next free handle; 0 otherwise
	 */
	static byte findFreeHandle();

	/**
	 * \brief Executes the entry by the given index. Do not call directly.
	 *
	 * \param index The index of the entry to execute
	 *
	 * \return count of milliseconds to next run if the entry úpdated its schedule to run again; ~0 otherwise.
	 */
	static word executeEntry(byte index);
};

#endif
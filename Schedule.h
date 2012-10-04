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

/** \brief Internally used only. \internal Execute once, parameters in milliseconds */
#define SCHEDULE_TYPE_MS_ONCE			1

/** \brief Internally used only. \internal Execute periodic, parameters in milliseconds */
#define SCHEDULE_TYPE_MS_PERIODIC 		2

/** \brief Internally used only. \internal Marker bit event handler requires handler parameter */
#define SCHEDULE_CALLBACK_WITH_HANDLE 	64

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
	 * \brief Register a callback that is executed after the given time in ms
	 *	 
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction()	 
	 * \returns a handle to the task entry on success; 0 otherwise
	 */
	static byte after(word ms, void (*callback)());
	
	/**
	 * \brief Register a callback that is executed after the given time in ms
	 *
	 * \param ms Delay in milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \returns a handle to the task entry on success; 0 otherwise
	 */
	static byte after(word ms, void (*callback)(byte));
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction()
	 * \param startDelay Delay in milliseconds
	 * \returns a handle to the task entry on success; 0 otherwise
	 */
	static byte every(word ms, void (*callback)(), word startDelay = -1);
	
	/**
	 * \brief Register a callback that is executed periodically
	 *
	 * \param ms Repeat every ms milliseconds
	 * \param callback Callback of type void myCallbackFunction(byte handle). The paramater handle specifies the handle of the schedule entry.	 
	 * \param startDelay Delay in milliseconds
	 * \returns a handle to the task entry on success; 0 otherwise
	 */
	static byte every(word ms, void (*callback)(byte), word startDelay = -1);
	
	/**
	 * \brief Removes the event with the specified handle from schedule
	 */
	static void remove(byte handle);
private:
	/**
	 * \brief Struct for schedule entries
	 */
	struct ScheduleEntry {
		byte scheduleType;
		word period;
		word next;
		void (*scheduleEvent)();
	};
	
	/**
	 * \brief Storage for the tasks
	 */
	static ScheduleEntry entries[MAX_SCHEDULE_ENTRIES];
	
	/**
	 * \brief Returns the next free handle; 0 otherwise
	 */
	static byte findFreeHandle();

};

#endif
/*
  Schedule.h - Library for communicating with wireless nodes.  
  Created by SimonJee, October 2, 2012.
  Released into the public domain.
*/

#ifndef Schedule_h
#define Schedule_h

#ifndef MAX_SCHEDULE_ENTRIES 
#define MAX_SCHEDULE_ENTRIES 10
#endif

#define SCHEDULE_TYPE_MS_ONCE			1
#define SCHEDULE_TYPE_MS_PERIODIC 		2
#define SCHEDULE_CALLBACK_WITH_HANDLE 	64

/**
 * \brief Static class for handling scheduled tasks.
 *
 */
class Schedule {
public:
	/**
	 * \brief Evaluates the schedule plan and goes to sleep mode
	 */
	static void loopAndSleep();
	
	/**
	 * \brief Evaluates the schedule plan
	 */
	 static void loop();
	 
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
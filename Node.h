/**
  \file Node.h  
  \brief Library for communicating with wireless nodes.  
  \author SimonJee
  \date Created on October 2, 2012.
  \copyright Released into the public domain.
*/

#ifndef Node_h
#define Node_h

#include <avr/pgmspace.h>

#include "Arduino.h"

#include "Schedule.h"

/**
 * The required idle mode
 */
enum IdleMode {		
	POWER_OFF 	= 1,  /** Nothing to do in the future */
	DEEP_SLEEP 	= 2,  /** Nothing to do until watchdog event */	
	SLEEP 		= 4,  /** Got to sleep but be prepared to react in under 64 msec */
	IDLE 		= 64  /** I've something to do in the next loop, just idle */
};

/**
 * \brief Callback type for wireless events
 */
typedef void (*nodeEvent)(byte eventType);

/**
 *	The node class offers high level functionality for wireless nodes
 */
class Node {
public:
	/**
	 * The connection options
	 */
	enum ConnectionOptions {
		NONE = 0, 
		ALLOW_REPAIRING = 1, 
		AUTO_RECONNECT = 2
	};
	
	/**
	 * Load the configuration from the eeprom.	 
	 */
	static void loadConfig();
	
	/**
	 * Sets the name of the node. Should be readable by humans.
	 */
	static void setName(const char* name);
	
	/**
	 * Try to connect controller asynchronously
	 */
	static void beginConnectToController(ConnectionOptions options);
	
	/**
	 * Cancel the connection
	 */
	static void cancelConnectionToController(ConnectionOptions options);
		
	/**
	 * Sets the connection state changed handler
	 */		
	static void setConnectionStateChangedHandler(nodeEvent handler);
	
	/**
	 * The loop function should be called from the main loop.
	 */	
	static IdleMode loop();
	
private:
	/**
	 * Stores the connection state changed handler
	 */
	static nodeEvent connectionStateChanged;
	
	/**
	 * Holds the name of the node	
	 */
	static const char* nodeName;
	
	/**
	 * Node id of controller if connected
	 */
	static byte controllerNodeId;
	
	
	/**
	 * Raises the event that the connection state has changed
	 */
	static void raiseOnConnectionStateChanged();
	
};
 
 #endif // Node_h
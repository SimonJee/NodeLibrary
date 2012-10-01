/*
  Node.h - Library for communicating with wireless nodes.  
  Created by SimonJee, October 1, 2012.
  Released into the public domain.
*/

#ifndef Node_h
#define Node_h

#include <avr/pgmspace.h>

#include "Arduino.h"

class Node {
public:
	static void setName(prog_char* PROGMEM name);
	
private:
	static prog_char* PROGMEM nodeName;
};

#endif // Node_h
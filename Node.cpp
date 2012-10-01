/*
  Node.cpp - Library for communicating with wireless nodes.
  Created by SimonJee, October 1, 2012.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Node.h"

prog_char* PROGMEM Node::nodeName = NULL;

void Node::setName(prog_char* PROGMEM name) {
	Node::nodeName = name;
}
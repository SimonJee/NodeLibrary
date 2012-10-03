/*
  Node.cpp - Library for communicating with wireless nodes.
  Created by SimonJee, October 1, 2012.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Node.h"

const char*  Node::nodeName = NULL;
byte Node::controllerNodeId = 0;
nodeEvent Node::connectionStateChanged = NULL;

void Node::setName(const char* name) {
	Node::nodeName = name;
}

void Node::loadConfig() {
}

void Node::beginConnectToController(ConnectionOptions options) {
	// Send message to controller, wait for ack
	// rf_12(....)
}

void Node::setConnectionStateChangedHandler(nodeEvent handler) {
	connectionStateChanged = handler;
}

void Node::raiseOnConnectionStateChanged() {
	if(connectionStateChanged) {
		// TODO: Fix constant type
		connectionStateChanged(0);
	}
}

IdleMode Node::loop() {
	//if(rf_12_revc ... {
	
	// Received ack from controller
	if(0) {
		
	}
	//}
	
	return IDLE;
}
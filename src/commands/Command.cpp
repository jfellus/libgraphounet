/*
 * Command.cpp
 *
 *  Created on: 11 nov. 2014
 *      Author: jfellus
 */

#include "Command.h"

Command::Command() {
	CommandStack::add(this);
}

std::list<Command*> CommandStack::undoables;
std::list<Command*> CommandStack::redoables;

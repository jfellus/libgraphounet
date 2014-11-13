/*
 * Command.h
 *
 *  Created on: 11 nov. 2014
 *      Author: jfellus
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "../util/utils.h"
#include <list>


class Command {
public:
	Command();
	virtual ~Command() {}

	virtual void execute() = 0;
	virtual void undo() = 0;

	virtual std::string get_text() = 0;
};

class CommandStack {
public:
	class Listener {
	public:
		Listener() {}
		virtual ~Listener() {}
		virtual void on_command_stack_change()  = 0;
	};
public:
	static std::list<Command*> undoables;
	static std::list<Command*> redoables;
	static std::vector<Listener*> listeners;
public:

	static void add_listener(Listener* l) {listeners.push_back(l);}

	static void add(Command* c) {
		undoables.push_back(c);
		while(!redoables.empty()) {
			Command *c = redoables.front();  redoables.pop_front();
			delete c;
		}
		for(uint i=0; i<listeners.size(); i++) listeners[i]->on_command_stack_change();
	}

	static void redo() {
		if(!can_redo()) return;
		Command* c = redoables.back(); redoables.pop_back();
		undoables.push_back(c);
		c->execute();
		for(uint i=0; i<listeners.size(); i++) listeners[i]->on_command_stack_change();
	}

	static void undo() {
		if(!can_undo()) return;
		Command* c = undoables.back(); undoables.pop_back();
		redoables.push_back(c);
		c->undo();
		for(uint i=0; i<listeners.size(); i++) listeners[i]->on_command_stack_change();
	}

	static bool can_redo() {return !redoables.empty();}
	static bool can_undo() {return !undoables.empty();}

	static std::string get_cur_redo() {
		if(!can_redo()) return "";
		return redoables.back()->get_text();
	}

	static std::string get_cur_undo() {
		if(!can_undo()) return "";
		return undoables.back()->get_text();
	}

	static void dump() {
		DBG("undoables " << undoables << "   |  redoables " << redoables);
	}
};


#endif /* COMMAND_H_ */

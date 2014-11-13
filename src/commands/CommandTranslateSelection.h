/*
 * CommandTranslateSelection.h
 *
 *  Created on: 11 nov. 2014
 *      Author: jfellus
 */

#ifndef COMMANDTRANSLATESELECTION_H_
#define COMMANDTRANSLATESELECTION_H_

#include "Command.h"
#include "../ZoomableDrawingArea.h"
#include "../Component.h"

class CommandTranslateSelection : public Command {
public:
	ZoomableDrawingArea* canvas;
	std::vector<Component*> selection;
	double dx,dy;
public:
	CommandTranslateSelection(ZoomableDrawingArea* canvas, double dx, double dy) :
		canvas(canvas), selection(canvas->selection), dx(dx), dy(dy) {
	}

	virtual ~CommandTranslateSelection() {}

	virtual void execute() {
		for(uint i=0; i<selection.size(); i++) {
			selection[i]->translate(dx,dy);
		}
		canvas->grab_focus();
		canvas->repaint();
	}

	virtual void undo() {
		for(uint i=0; i<selection.size(); i++) {
			selection[i]->translate(-dx,-dy);
		}
		canvas->grab_focus();
		canvas->repaint();
	}

	virtual std::string get_text() {
		return "translate";
	}
};

#endif /* COMMANDTRANSLATESELECTION_H_ */

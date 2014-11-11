/*
 * Creator.h
 *
 *  Created on: 6 oct. 2014
 *      Author: jfellus
 */

#ifndef CREATOR_H_
#define CREATOR_H_

#include "../Component.h"

class Creator : public Component {
protected:
	Component* comp;

public:
	Creator();
	virtual ~Creator() {}


	// Lifecycle

	virtual void start(ZoomableDrawingArea* canvas);
	virtual void create(double x, double y);
	virtual void end();


	// Rendering

	virtual void render(Graphics& g) = 0;
	virtual void transform(Graphics& g) {} // No transform !


	// Events

	virtual void on_mouse_move(GdkEventMotion* e);
	virtual void on_click(GdkEventButton* e);
	virtual void on_unclick(GdkEventButton* e);



	// Utils

	virtual void dump(std::ostream& os) { os << "Creator(" << comp << ")";}

};

#endif /* CREATOR_H_ */

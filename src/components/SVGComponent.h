/*
 * SVGComponent.h
 *
 *  Created on: 2 oct. 2014
 *      Author: jfellus
 */

#ifndef SVGCOMPONENT_H_
#define SVGCOMPONENT_H_

#include "../Component.h"
#include "../graphics/svg.h"
#include "../style/SVGDefinitions.h"


class SVGComponent : public Component {
public:
	SVG* svg = NULL;
	Rectangle bounds;
public:
	SVGComponent(const char* filename) {
		ready = false;
		set(filename);
	}
	virtual ~SVGComponent() {}


	void set(const char* filename);

	void load(const char* filename);


	virtual Rectangle get_bounds() {
		if(!bounds) compute_bounds();
		return bounds + Vector2D(x,y);
	}

	Vector2D get_pos() {return Vector2D(x,y);}

	virtual void render(Graphics& g);

	virtual void select(bool single) {
		Component::select(single);
	}

protected:
	virtual void compute_bounds() {
		//if(ready)
		CAIRO_THREAD_SAFE(bounds = svg->get_bounds());
	}

	virtual void dump(std::ostream& os) { os << "SVGComponent(" << svg->filename << ")";}
};


#endif /* SVGCOMPONENT_H_ */

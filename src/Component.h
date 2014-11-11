/*
 * Component.h
 *
 *  Created on: 2 oct. 2014
 *      Author: jfellus
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "util/geom.h"
#include "graphics/Graphics.h"
#include <gtk/gtk.h>
#include "util/selection/ISelectable.h"
#include "style/IStyle.h"

class ZoomableDrawingArea;
class BoundingBox;

class Component : public ISelectable {
public:

	double x,y;

	bool visible = false;
	int selectionLayer = 0;
	float layer = 0;


protected:

	ZoomableDrawingArea* canvas;
	BoundingBox* selectionBox = 0;
	bool auto_addToCanvas = true;

	IStyle* style = NULL;
	std::string css_class;



public:

	Component(int layer);
	Component(bool auto_addToCanvas = true);
	virtual ~Component();


	// Accessors

	virtual Vector2D center() { return get_bounds().center();}

	void set_selectable(bool b = true);
	virtual void set_canvas(ZoomableDrawingArea* c) {canvas = c;}

	virtual Rectangle get_bounds() { return Rectangle();}
	virtual bool hasPoint(double x, double y) {return get_bounds().contains(x,y); }
	virtual bool is_in(const Rectangle& r) { return get_bounds().is_in(r); }


	// Transformation

	virtual void show() { visible=true; }
	virtual void hide() { visible=false; }

	void set_pos(double x, double y) 	{ this->x = x; this->y = y; }
	void set_pos(const Vector2D& p) 	{ set_pos(p.x, p.y);}
	void center(const Vector2D& p) 		{ Rectangle r = get_bounds(); set_pos(p.x - r.w/2, p.y - r.h/2);}
	virtual void translate(double dx, double dy) {this->x += dx; this->y += dy; 	}

	virtual void select(bool single);
	virtual void unselect();
	virtual void click(double x, double y) {}


	// CSS

	void add_class(const std::string& c);
	void remove_class(const std::string& c);
	bool has_class(const std::string& c);
	void toggle_class(const std::string& c) {if(has_class(c)) remove_class(c); else add_class(c);}


	// Rendering

	virtual void render(Graphics& g) = 0;
	virtual void transform(Graphics& g) {g.translate(x,y);}
	virtual void draw(Graphics& g);


	// User data

	struct user_data {std::string type_info; void* p;};
	struct user_data user_data;
	void set_user_data(const std::string& type_info, void* data) {user_data.p = data; user_data.type_info = type_info;}
	void* get_user_data(const std::string& type_info) {return user_data.type_info==type_info ? user_data.p : NULL;}


	// Utils

	virtual void dump(std::ostream& os) { os << "Component(" << (void*)this << ")";}
	friend std::ostream& operator<<(std::ostream& os, Component* a);
};

inline std::ostream& operator<<(std::ostream& os, Component* a) {a->dump(os);return os;}


#endif /* COMPONENT_H_ */

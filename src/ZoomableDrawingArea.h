/*
 * ZoomableDrawingArea.h
 *
 *  Created on: 1 oct. 2014
 *      Author: jfellus
 */

#ifndef ZOOMABLEDRAWINGAREA_H_
#define ZOOMABLEDRAWINGAREA_H_

#include <gtk/gtk.h>
#include "graphics/svg.h"
#include "Component.h"
#include "creator/Creator.h"
#include <vector>
#include <algorithm>
#include "widget/Widget.h"
#include "util/events/listeners.h"

class Component;
class BoundingBox;



class ZoomableDrawingArea : public Widget {
public:
	///////////////
	// LISTENERS //
	///////////////

	class ISelectionListener {
	public:
		ISelectionListener() {}
		~ISelectionListener() {}
		virtual void on_selection_change() = 0;
	};

	class IChangeListener {
	public:
		IChangeListener() {}
		~IChangeListener() {}
		virtual void on_canvas_change() = 0;
	};

	///////////
	// ENUMS //
	///////////

	enum MoveMode {	MOVE_MODE_SCROLL, MOVE_MOUSE_MIDDLE_BUTTON	};
	enum SelectionRenderingMode {
		SELECTION_ShowComponentsBoundingBoxes,
		SELECTION_ShowSingleBoundingBox,
		SELECTION_Colored,
	};



public:

	////////////
	// FIELDS //
	////////////


	Vector2D mousePosDoc;

	double _zoom;
	double offsetx, offsety;

	Creator* creator = 0;

	BoundingBox* selectionBox = 0;
	BoundingBox* multi_selectionBox = 0;
	Component* draggedComponent = 0;

	enum MoveMode moveMode = MOVE_MOUSE_MIDDLE_BUTTON;
	enum SelectionRenderingMode selectionRenderingMode = SELECTION_Colored;


	// Event listeners
	std::vector<Component*> components;
	std::vector<Component*> components_selectables;
	std::vector<Component*> components_topified;
	std::vector<Component*> selection;
	std::vector<IKeyListener*> keylisteners;
	std::vector<IMouseListener*> mouseListeners;
	std::vector<ISelectionListener*> selectionListeners;
	std::vector<IChangeListener*> changeListeners;


	// Internals
	bool isSelecting = false;
	double drag_start_x = 0, drag_start_y = 0;
	bool bFirstDraw = true;
	bool isDragging;
	double oldx_mouse=0, oldy_mouse=0, oldx=0, oldy=0;




public:

	ZoomableDrawingArea(Widget* win);
	virtual ~ZoomableDrawingArea();
	static ZoomableDrawingArea* cur();


	// Accessors

	void add(Component* c);
	void remove(Component* c);

	void add_selectable(Component* c);
	void remove_selectable(Component* c);
	Component* get_selectable_component_at(double x, double y);

	double get_zoom() { return _zoom; }

	Vector2D get_mouse_pos_doc() {return mousePosDoc;}
	double mouse_to_doc_x(double x) {return (x-offsetx)/_zoom;}
	double mouse_to_doc_y(double y) {return (y-offsety)/_zoom;}
	double doc_to_mouse_x(double x) {return x*_zoom + offsetx; }
	double doc_to_mouse_y(double y) {return y*_zoom + offsety; }

	void add_key_listener(IKeyListener* kl) 			{keylisteners.push_back(kl);}
	void add_key_listener(IMouseListener* kl) 			{mouseListeners.push_back(kl);}
	void add_selection_listener(ISelectionListener* l) 	{selectionListeners.push_back(l);}
	void add_change_listener(IChangeListener* l) 		{changeListeners.push_back(l);}

	// Selection

	void select(Component* c, bool single);
	void select(double x1, double y1, double x2, double y2);
	void unselect(Component* c);
	void unselect_all();
	void toggle_select(Component* c);
	void select_all();
	void topify(Component* c);
	void untopify(Component* c);
	bool has_selection() {return !selection.empty();}


	// Navigation

	void zoom_all();
	void zoom(Component* c);
	void zoom(const Rectangle& r);
	void zoom(float fzoom, double cx, double cy);
	void zoom(float fzoom) {_zoom *= (1 + fzoom); repaint();}
	void zoom_reset() {	_zoom = 0.08;	offsetx = offsety = 0;	repaint(); }
	void move(double dx, double dy) {offsetx += dx; offsety += dy; repaint();}


	// Transformations

	void translate_selection(double dx, double dy);


	// Creators

	void start_creator(Creator* c);
	void end_creator();


	// Display

	virtual void repaint() {if(!isSelecting) Widget::repaint();}



	// ! Internals !

	void add_selection(Component* c);
	void remove_selection(Component* c);

	void update_layers();

protected:
	void fire_selection_change();
	void fire_change_event();
	friend struct _callbacks;
	virtual bool on_click(GdkEventButton* event);
	virtual bool on_dbl_click(GdkEventButton* event);
	virtual bool on_unclick(GdkEventButton* event);
	virtual bool on_draw(cairo_t* cr);
	virtual bool on_key(GdkEventKey* event);
	virtual bool on_mouse_move(GdkEventMotion* event);
	virtual bool on_scroll(GdkEventScroll* event);
	virtual void on_first_draw();
public:
	virtual void on_realizes();
};



#endif /* ZOOMABLEDRAWINGAREA_H_ */

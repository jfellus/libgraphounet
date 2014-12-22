/*
 * ZoomableDrawingArea.cpp
 *
 *  Created on: 1 oct. 2014
 *      Author: jfellus
 */

#include "ZoomableDrawingArea.h"
#include "util/utils.h"
#include "components/BoundingBox.h"
#include <typeinfo>
#include "commands/CommandTranslateSelection.h"

///////////////
// CALLBACKS //
///////////////

typedef struct _callbacks {
	static gboolean _on_mouse_move (GtkWidget *widget, GdkEventMotion  *event, gpointer   user_data) {		return ((ZoomableDrawingArea*)user_data)->on_mouse_move(event);	}
	static gboolean _on_click (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {		return((ZoomableDrawingArea*)user_data)->on_click(event);	}
	static gboolean _on_unclick (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {		return ((ZoomableDrawingArea*)user_data)->on_unclick(event);	}
	static gboolean _on_scroll (GtkWidget *widget, GdkEventScroll  *event, gpointer   user_data) {		return ((ZoomableDrawingArea*)user_data)->on_scroll(event);	}
	static gboolean _on_realize (GtkWidget *widget, gpointer   user_data) {		return true;	}
	static gboolean _on_draw (GtkWidget    *widget, cairo_t *cr,  gpointer      user_data) {		return ((ZoomableDrawingArea*)user_data)->on_draw(cr);	}
	static gboolean _on_key (GtkWidget *widget, GdkEventKey  *event, gpointer   user_data) {		return ((ZoomableDrawingArea*)user_data)->on_key(event);	}
} _callbacks;

static void on_undo() { CommandStack::undo(); }
static void on_redo() { CommandStack::redo(); }
static void on_zoom(double x, double y, double dx, double dy) { ZoomableDrawingArea::cur()->on_zoom(x,y,dx,dy); }




//////////////////
// CONSTRUCTION //
//////////////////

static ZoomableDrawingArea* _cur = 0;
ZoomableDrawingArea* ZoomableDrawingArea::cur() {	return _cur; }


ZoomableDrawingArea::ZoomableDrawingArea(Widget* w) : Widget(gtk_drawing_area_new()){
	_cur = this;
	gtk_widget_show(widget);
	isDragging = false;
	_zoom = 1;
	offsetx = offsety = 0;
	draggedComponent = 0;

	gtk_widget_set_size_request(GTK_WIDGET(widget), 800,600);


	set_events(GDK_ALL_EVENTS_MASK);
	gtk_widget_set_can_focus(widget, TRUE);
	g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(_callbacks::_on_draw), this);
	g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(_callbacks::_on_scroll), this);
	g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(_callbacks::_on_mouse_move), this);
	g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(_callbacks::_on_click), this);
	g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(_callbacks::_on_unclick), this);
	g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(_callbacks::_on_key), this);
	g_signal_connect(G_OBJECT(widget), "realize", G_CALLBACK(_callbacks::_on_realize), this);
	grab_focus();

	selectionBox = new BoundingBox(&selection);
	multi_selectionBox = new BoundingBox(Rectangle());

	add_key_listener(new IKeyListener(GDK_KEY_z, GDK_CONTROL_MASK, on_undo));
	add_key_listener(new IKeyListener(GDK_KEY_Z, GDK_CONTROL_MASK | GDK_SHIFT_MASK, on_redo));

	add_scroll_listener(new IScrollListener(GDK_CONTROL_MASK, ::on_zoom));
}

ZoomableDrawingArea::~ZoomableDrawingArea() {}

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mut1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_t locker = 0;
static int nlocks = 0;
static bool bDrawEnabled = true;
static int nboff = 0;
void ZoomableDrawingArea::LOCK() {
	pthread_mutex_lock(&mut1);
	if(locker==pthread_self()) {nlocks++; pthread_mutex_unlock(&mut1); return;}
	pthread_mutex_unlock(&mut1);
	pthread_mutex_lock(&mut);
	locker = pthread_self();
//	f_write_line("locker.txt", GET_STACK_TRACE());
	nlocks++;
}
void ZoomableDrawingArea::UNLOCK() {
	nlocks--;
	if(nlocks<=0) {
		locker = 0;
		pthread_mutex_unlock(&mut);
	}
}
bool ZoomableDrawingArea::IS_LOCKED() {
	return locker==pthread_self();
}
void ZoomableDrawingArea::OFF() {
	LOCK();
	nboff++;
	bDrawEnabled = false;
	UNLOCK();
}
void ZoomableDrawingArea::ON() {
	LOCK();
	nboff--;
	if(nboff <= 0) {
		nboff = 0;
		bDrawEnabled = true;
	}
	UNLOCK();
}


void ZoomableDrawingArea::repaint() {
	if(bDrawEnabled && !isSelecting) Widget::repaint();
}


///////////////
// ACCESSORS //
///////////////


Component* ZoomableDrawingArea::get_selectable_component_at(double x, double y) {
	for(int i=components_topified.size()-1; i>=0; i--) {
		Component* c = components_topified[i];
		if(c->visible && !c->bLock && c->hasPoint(x,y)) return c;
	}
	for(int i=components_selectables.size()-1; i>=0; i--) {
		Component* c = components_selectables[i];
		if(c->visible && !c->bLock && c->hasPoint(x,y)) return c;
	}
	return 0;
}



/////////////
// METHODS //
/////////////

void ZoomableDrawingArea::add(Component* c) {
	LOCK();
	components.push_back(c);
	c->set_canvas(this);
	update_layers();
	UNLOCK();
}

void ZoomableDrawingArea::remove(Component* c) {
	LOCK();
	c->ready = false;
	remove_selectable(c);
	if(c->bSelected) remove_selection(c);
	vector_remove(components,c);
	if(c==curHover) curHover=NULL;
	c->set_canvas(0);
	UNLOCK();
}

void ZoomableDrawingArea::clear() {
	LOCK();
	unselect_all();
	while(!components_selectables.empty()) remove_selectable(components_selectables[0]);
	while(!components.empty()) {Component* c = components[0]; vector_remove(components, c); delete c;}
	curHover = NULL;
	UNLOCK();
}

////////////////
// NAVIGATION //
////////////////

void ZoomableDrawingArea::zoom(float fzoom, double cx, double cy) {
	LOCK();
	cx =  (cx - offsetx)/ _zoom ;
	cy =  (cy - offsety)/ _zoom ;
	double oldzoom = _zoom;
	_zoom *= 1 + fzoom;
	offsetx -= cx*(_zoom - oldzoom);
	offsety -= cy*(_zoom - oldzoom);
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::zoom(Component* c) {	zoom(c->get_bounds()); }

void ZoomableDrawingArea::zoom(const Rectangle& r) {
	LOCK();
	double w = get_width()-20;
	double h = get_height()-20;
	if(!w || !h) return;
	_zoom = MIN(h/r.h, w/r.w);

	offsetx= -r.x*_zoom + 10 + MAX(0,(w - r.w*_zoom)/2);
	offsety= -r.y*_zoom + 10 + MAX(0,(h - r.h*_zoom)/2);
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::zoom_all() {
	if(components.empty()) return;
	LOCK();
	Rectangle r = components[0]->get_bounds();
	for(uint i=1; i<components.size(); i++) {
		if(components[i]->visible) r.add(components[i]->get_bounds());
	}
	zoom(r);
	UNLOCK();
}



///////////////
// SELECTION //
///////////////

void ZoomableDrawingArea::select_all() {
	LOCK();
	isSelecting = true;
	for(uint i=0; i<components_selectables.size(); i++)
		if(components_selectables[i]->visible && !components_selectables[i]->bLock) components_selectables[i]->select(false);
	isSelecting = false;
	fire_selection_change();
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::select(Component* c, bool single) {
	LOCK();
	c->select(single);
	fire_selection_change();
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::unselect(Component* c)			{
	LOCK();
	c->unselect();
	fire_selection_change();
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::toggle_select(Component* c) 		{
	LOCK();
	c->toggle_select();
	fire_selection_change();
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::select(double x1, double y1, double x2, double y2) {
	LOCK();
	isSelecting = true;
	Rectangle r(x1,y1,x2-x1,y2-y1);
	for(uint i=0; i<components_topified.size(); i++) {
		Component* c = components_topified[i];
		if(!c->visible || c->bLock) continue;
		if(c->is_in(r) && !c->is_selected()) c->select(false);
	}
	for(uint i=0; i<components_selectables.size(); i++) {
		Component* c = components_selectables[i];
		if(!c->visible || c->bLock) continue;
		if(c->is_in(r) && !c->is_selected()) c->select(false);
	}
	isSelecting = false;
	fire_selection_change();
	UNLOCK();
	repaint();
}


void ZoomableDrawingArea::unselect_all() {
	LOCK();
	isSelecting = true;
	while(!components_topified.empty()) untopify(components_topified[0]);
	while(!selection.empty()) selection[0]->unselect();
	selectionBox->set_bounds(Rectangle());
	isSelecting = false;
	fire_selection_change();
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::add_selection(Component* c) {
	LOCK();
	selection.push_back(c);
	selectionBox->set_bounds(&selection);
	UNLOCK();
}

void ZoomableDrawingArea::remove_selection(Component* c) {
	LOCK();
	vector_remove(selection, c);
	selectionBox->set_bounds(&selection);
	if(selection.empty()) selectionBox->set_bounds(Rectangle());
	UNLOCK();
}




/////////////////////
// TRANSFORMATIONS //
/////////////////////

void ZoomableDrawingArea::translate_selection(double dx, double dy) {
	LOCK();
	for(uint i=0; i<selection.size(); i++) {
		selection[i]->translate(dx,dy);
	}
	grab_focus();
	UNLOCK();
	repaint();
}



////////////
// LAYERS //
////////////

static bool _drawingLayerComp(Component* c1, Component* c2) {	return c1->layer < c2->layer;}
static bool _selectionLayerComp(Component* c1, Component* c2) {	return c1->selectionLayer < c2->selectionLayer;}

void ZoomableDrawingArea::update_layers() {
	LOCK();
	std::sort (components.begin(), components.end(), _drawingLayerComp);
	UNLOCK();
	repaint();
}

void ZoomableDrawingArea::add_selectable(Component* c) {
	LOCK();
	components_selectables.push_back(c);
	std::sort (components_selectables.begin(), components_selectables.end(), _selectionLayerComp);
	UNLOCK();
}

void ZoomableDrawingArea::remove_selectable(Component* c) {vector_remove(components_selectables,c);}

void ZoomableDrawingArea::topify(Component* c) {	components_topified.push_back(c);}
void ZoomableDrawingArea::untopify(Component* c) {	vector_remove(components_topified,c);}



//////////////
// CREATORS //
//////////////

static Creator* _do_start_creator_creator = NULL;
static int _do_start_creator(void* p) {
	ZoomableDrawingArea* canvas = (ZoomableDrawingArea*)p;
	if(canvas->creator!=NULL) {canvas->creator->end(); canvas->end_creator();}
	canvas->creator = _do_start_creator_creator; _do_start_creator_creator->start(canvas);
	canvas->repaint();
	return FALSE;
}
void ZoomableDrawingArea::start_creator(Creator* c) {
	_do_start_creator_creator = c;
	g_timeout_add(1, _do_start_creator, this);
}

void ZoomableDrawingArea::end_creator() {
	delete creator;
	creator = NULL;
	repaint();
}



////////////
// EVENTS //
////////////

void ZoomableDrawingArea::on_realizes() {
	if(!bDrawEnabled) return;
	grab_focus();
	zoom_all();
}

bool ZoomableDrawingArea::on_key(GdkEventKey* event) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	if(event->state & GDK_CONTROL_MASK) {
		if(event->keyval==GDK_KEY_a) select_all();

		double a = 10;
		if(event->state & GDK_SHIFT_MASK) a *= 5;
		if(event->keyval==GDK_KEY_Left) move(a,0);
		if(event->keyval==GDK_KEY_Right) move(-a,0);
		if(event->keyval==GDK_KEY_Up) move(0,a);
		if(event->keyval==GDK_KEY_Down) move(0, -a);
	} else {
		if(event->keyval==GDK_KEY_z) zoom_all();

		double a = 3;
		if(event->state & GDK_SHIFT_MASK) a *= 5;
		double dx = 0, dy = 0;
		if(event->keyval==GDK_KEY_Left) 	dx = -a/_zoom;
		if(event->keyval==GDK_KEY_Right) 	dx = a/_zoom;
		if(event->keyval==GDK_KEY_Up) 		dy = -a/_zoom;
		if(event->keyval==GDK_KEY_Down) 	dy = a/_zoom;
		if(dx!=0 || dy!=0) {
			(new CommandTranslateSelection(this,dx,dy))->execute();
			 fire_change_event();
		}
	}

	for(uint i=0; i<keylisteners.size(); i++) {
		IKeyListener* kl = keylisteners[i];
		if(event->keyval==kl->key && ((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_SUPER_MASK)) == kl->mask)) kl->on_key();
	}

	grab_focus();
	UNLOCK();
	return TRUE;
}

bool ZoomableDrawingArea::on_dbl_click(GdkEventButton* e) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	if(!selection.empty()) selection[0]->on_dbl_click(e);
	for(uint i=0; i<dblclickListeners.size(); i++) dblclickListeners[i]->on_dbl_click(selection.empty() ? NULL : selection[0]);
	UNLOCK();
	return false;
}


bool ZoomableDrawingArea::on_click(GdkEventButton* event) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	if(event->type == GDK_2BUTTON_PRESS) on_dbl_click(event);
	else if(creator) {
		creator->on_click(event);
		UNLOCK();
		return true;
	}
	else if(event->button==1) {
		drag_start_x = mouse_to_doc_x(event->x);
		drag_start_y = mouse_to_doc_y(event->y);

		if(event->state & GDK_SHIFT_MASK) draggedComponent = 0;
		else {
			draggedComponent = get_selectable_component_at(drag_start_x, drag_start_y);
			if(draggedComponent) draggedComponent->click(drag_start_x, drag_start_y);
			if(!draggedComponent || !draggedComponent->is_selected()) {
				unselect_all();
				if(draggedComponent) select(draggedComponent, true);
			}
		}
	}

	grab_focus();
	UNLOCK();
	return true;
}

bool ZoomableDrawingArea::on_unclick(GdkEventButton* event) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	mousePosDoc.x = mouse_to_doc_x(event->x);
	mousePosDoc.y = mouse_to_doc_y(event->y);

	if(creator) {
		creator->on_unclick(event);
		repaint();
	}

	else if(event->button==1) {
		multi_selectionBox->set_bounds(Rectangle());

		if(isDragging && draggedComponent && has_selection()) {
			new CommandTranslateSelection(this, mousePosDoc.x - drag_start_x, mousePosDoc.y - drag_start_y);
			for(uint i=0; i<selection.size(); i++) selection[i]->translate(0,0, true);
		}

		if(isDragging && !draggedComponent) {
			select(drag_start_x, drag_start_y, mousePosDoc.x, mousePosDoc.y);
		}

		if(!isDragging && (event->state & GDK_SHIFT_MASK)) {
			draggedComponent = get_selectable_component_at(mousePosDoc.x, mousePosDoc.y);
			if(draggedComponent) toggle_select(draggedComponent);
		}

		if(isDragging) fire_change_event();

		draggedComponent = 0;
		isDragging = false;
		repaint();
	}

	grab_focus();
	UNLOCK();
	return true;
}



bool ZoomableDrawingArea::on_mouse_move(GdkEventMotion* event) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	mousePosDoc.x = mouse_to_doc_x(event->x);
	mousePosDoc.y = mouse_to_doc_y(event->y);

	if((event->state & GDK_BUTTON2_MASK) && (moveMode == MOVE_MOUSE_MIDDLE_BUTTON)) {
		move(event->x - oldx_mouse, event->y - oldy_mouse);
	}

	else if(creator) {
		creator->on_mouse_move(event);
		repaint();
	}

	else if(event->state & GDK_BUTTON1_MASK) {
		isDragging = true;
		if(has_selection() && !(event->state & GDK_SHIFT_MASK)) {
			translate_selection(mousePosDoc.x-oldx, mousePosDoc.y-oldy);
		} else {
			multi_selectionBox->set_bounds(Rectangle(drag_start_x, drag_start_y,mousePosDoc.x-drag_start_x, mousePosDoc.y-drag_start_y));
			repaint();
		}
	}

	Component* hover = get_selectable_component_at(mousePosDoc.x, mousePosDoc.y);
	if(hover!=curHover){
		for(uint i=0; i<hoverListeners.size(); i++) hoverListeners[i]->on_hover(hover);
		if(curHover) curHover->remove_class("hover");
		if(hover) hover->add_class("hover");
		curHover = hover;
		repaint();
	}

	oldx = mousePosDoc.x; oldy = mousePosDoc.y;
	oldx_mouse = event->x; oldy_mouse = event->y;
	for(uint i=0; i<mouseListeners.size(); i++) mouseListeners[i]->on_mouse_move(mousePosDoc);
	UNLOCK();
	return true;
}

bool ZoomableDrawingArea::on_scroll(GdkEventScroll* event) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	double dx, dy;
	gdk_event_get_scroll_deltas((const GdkEvent*)event, &dx, &dy);

	for(uint i=0; i<scrolllisteners.size(); i++) {
		IScrollListener* kl = scrolllisteners[i];
		if(((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_SUPER_MASK)) == kl->mask)) kl->on_scroll(event->x, event->y, dx, dy);
	}
	UNLOCK();
	return true;
}

void ZoomableDrawingArea::on_zoom(double x, double y, double dx, double dy) {
	if(!bDrawEnabled) return;
	LOCK();
	zoom(-0.1 * dy, x, y);
	UNLOCK();
}

void ZoomableDrawingArea::on_first_draw() {
	LOCK();
	zoom_all();
	bFirstDraw = false;
	UNLOCK();
	pthread_setname_np(pthread_self(), "Drawing");
}

bool ZoomableDrawingArea::on_draw(cairo_t* cr) {
	if(!bDrawEnabled) return FALSE;
	LOCK();
	if(bFirstDraw) on_first_draw();
	Graphics g(cr, this);
	g.fill_all(RGB_WHITE);
	g.translate(offsetx, offsety);
	g.scale(_zoom);

	g.set_color(RGB_BLACK);
	for(uint i=0; i<components.size(); i++) components[i]->draw(g);

	if(selectionRenderingMode==SELECTION_ShowSingleBoundingBox && selectionBox) selectionBox->draw(g);
	if(multi_selectionBox) multi_selectionBox->draw(g);
	if(creator) creator->draw(g);
	UNLOCK();

	return FALSE;
}

void ZoomableDrawingArea::fire_selection_change() {
	for(uint i=0; i<selectionListeners.size(); i++) selectionListeners[i]->on_selection_change();
}

void ZoomableDrawingArea::fire_change_event() {
	for(uint i=0; i<changeListeners.size(); i++) changeListeners[i]->on_canvas_change();
}




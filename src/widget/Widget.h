/*
 * Widget.h
 *
 *  Created on: 8 oct. 2014
 *      Author: jfellus
 */

#ifndef WIDGET_H_
#define WIDGET_H_

#include <gtk/gtk.h>
#include "../util/geom.h"


class Widget {
public:
	GtkWidget* widget;
public:
	Widget(GtkWidget* w) { this->widget = w; }
	virtual ~Widget() {}

	void add(Widget* w) {gtk_container_add(GTK_CONTAINER(widget), w->widget);}
	void add(GtkWidget* w) {gtk_container_add(GTK_CONTAINER(widget), w);}

	void set_events(guint events) {gtk_widget_set_events(widget, events);}
	int get_width() {return gtk_widget_get_allocated_width(widget);}
	int get_height() {return gtk_widget_get_allocated_height(widget);}
	Vector2D get_absolute_position() {	gint wx, wy;	gdk_window_get_origin (gtk_widget_get_window (widget), &wx, &wy);	return Vector2D(wx,wy);	}

	virtual void repaint() {if(GTK_IS_WIDGET(widget)) gtk_widget_queue_draw(widget);}
	void grab_focus() {if(GTK_IS_WIDGET(widget)) gtk_widget_grab_focus(widget);}

	operator GtkWidget*() {return widget;}
};

#endif /* WIDGET_H_ */

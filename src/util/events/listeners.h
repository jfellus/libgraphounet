/*
 * listeners.h
 *
 *  Created on: 9 nov. 2014
 *      Author: jfellus
 */

#ifndef LISTENERS_H_
#define LISTENERS_H_


class IKeyListener {
public:
	void (*fn)(void);
	guint key; guint mask;
public:
	IKeyListener(int key, int mask, void (*fn)(void)){this->key = key; this->mask = mask; this->fn = fn;}
	virtual ~IKeyListener(){}
	void on_key() {fn();}
};

class IMouseListener {
public:
	IMouseListener() {}
	virtual ~IMouseListener(){}
	virtual void on_mouse_move(const Vector2D& docPos) {}
};

class IScrollListener {
public:
	void (*fn)(double x, double y, double dx, double dy, void* data);
	guint mask;
	void* data;
public:
	IScrollListener(int mask, void (*fn)(double, double, double, double, void*), void* data){this->mask = mask; this->fn = fn;this->data = data;}
	virtual ~IScrollListener(){}
	virtual void on_scroll(double x, double y, double dx, double dy) {fn(x,y,dx,dy, data);}
};



#endif /* LISTENERS_H_ */

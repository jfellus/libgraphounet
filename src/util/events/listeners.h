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



#endif /* LISTENERS_H_ */

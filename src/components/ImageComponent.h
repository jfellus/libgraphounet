/*
 * ImageComponent.h
 *
 *  Created on: 27 déc. 2014
 *      Author: jfellus
 */

#ifndef IMAGECOMPONENT_H_
#define IMAGECOMPONENT_H_

#include "../Component.h"

class ImageComponent : public Component {
public:
	unsigned char* data = NULL;
	bool bRGB = false;

	int width = 0, height = 0;

public:
	ImageComponent(bool bAuto_add_to_canvas = true);
	virtual ~ImageComponent();

	void set_width(int w) { this->width = w; }
	void set_height(int h) { this->height = h; }

	void set(float* data);
	void set(unsigned char* data);

	bool is_rgb() {return bRGB;}
	bool is_grayscale() {return !bRGB;}

	virtual Rectangle get_bounds();

	virtual void render(Graphics& g);
};

#endif /* IMAGECOMPONENT_H_ */

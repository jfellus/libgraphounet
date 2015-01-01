/*
 * ImageComponent.cpp
 *
 *  Created on: 27 déc. 2014
 *      Author: jfellus
 */

#include "ImageComponent.h"

ImageComponent::ImageComponent(bool bAuto_add_to_canvas) : Component(bAuto_add_to_canvas){
	ready = false;
	x=y=0;
}

ImageComponent::~ImageComponent() {
}

void ImageComponent::set(float* data) {
	this->data = (unsigned char*)data;
	bRGB = false;
	ready = true;
}

void ImageComponent::set(unsigned char* data) {
	this->data = data;
	bRGB = true;
	ready = true;
}

Rectangle ImageComponent::get_bounds() {
	return Rectangle(x,y,width,height);
}

void ImageComponent::render(Graphics& g) {
	if(bRGB) g.drawImageRGB((unsigned char*)data, width, height);
	else g.drawImageGrayscale((float*)data, width, height);
}



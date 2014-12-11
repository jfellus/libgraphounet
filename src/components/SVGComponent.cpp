/*
 * SVGComponent.cpp
 *
 *  Created on: 9 déc. 2014
 *      Author: jfellus
 */




#include "SVGComponent.h"
#include "../ZoomableDrawingArea.h"



void SVGComponent::load(const char* filename) {
	DBG("Load " << filename);
	try {
		svg = SVG::get_resource(filename);
		get_bounds();
		ready = true;
	} catch(...) {
		ERROR("Couldn't load SVG file : " << filename); throw "";
	}
	DBG("end load");
}


void SVGComponent::set(const char* filename) {
	DBG("Set  " << filename);
	if(file_has_ext(filename, ".svg")) load(filename);
	else {
		std::string f = SVGDefinitions::get(filename);
		if(f.empty()) { throw "";}
		else load(f.c_str());
	}
	bounds = Rectangle();
	DBG("End set");
}

void SVGComponent::render(Graphics& g) {
	DBG("render");
	if(svg) g.drawSVG(*svg);
	DBG("end render");
}


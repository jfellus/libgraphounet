/*
 * Component.cpp
 *
 *  Created on: 2 oct. 2014
 *      Author: jfellus
 */



#include "ZoomableDrawingArea.h"
#include "Component.h"
#include "components/BoundingBox.h"


//////////////////
// CONSTRUCTION //
//////////////////

Component::Component(int layer) {
	this->auto_addToCanvas = true;
	 x=y=0; canvas = 0;
	 selectionBox = 0;
	 this->layer = layer;
	 if(ZoomableDrawingArea::cur()) ZoomableDrawingArea::cur()->add(this);
	 visible = true;
	 ready = true;
}

Component::Component(bool auto_addToCanvas) {
	this->auto_addToCanvas = auto_addToCanvas;
	 x=y=0; canvas = 0;
	 selectionBox = 0;
	 if(auto_addToCanvas && ZoomableDrawingArea::cur()) ZoomableDrawingArea::cur()->add(this);
	 visible = true;
}

Component::~Component() {
	ready = false;
	if(!canvas) return;
	if(auto_addToCanvas) canvas->remove(this);
}



///////////////
// SELECTION //
///////////////


void Component::select(bool single) {
	if(bSelected) return;
	if(!selectionBox && canvas->selectionRenderingMode==ZoomableDrawingArea::SELECTION_ShowSingleBoundingBox) selectionBox = new BoundingBox(this);
	canvas->add_selection(this);
	ISelectable::select();
}

void Component::unselect() {
	if(!bSelected) return;
	canvas->remove_selection(this);
	ISelectable::unselect();
}

void Component::set_selectable(bool b) {
	if(b) canvas->add_selectable(this);
	else canvas->remove_selectable(this);
}



///////////////
// RENDERING //
///////////////

void Component::draw(Graphics& g) {
	if(!visible || !ready) return;

	Graphics::lock();
		g.save();
		g.clear_path();
		transform(g);
		if(canvas && canvas->selectionRenderingMode==ZoomableDrawingArea::SELECTION_Colored && is_selected())
			g.map_color(RGB_BLACK, RGB_RED);
		render(g);
			g.unmap_color(RGB_BLACK);
		g.restore();
		if(canvas && canvas->selectionRenderingMode==ZoomableDrawingArea::SELECTION_ShowComponentsBoundingBoxes && is_selected())
			selectionBox->draw(g);
	Graphics::unlock();
}




/////////
// CSS //
/////////

bool Component::has_class(const std::string& c) {
	return css_class.find(std::string(" ") + c + " ") != std::string::npos;
}

void Component::add_class(const std::string& c) {
	if(css_class.empty()) css_class = " " + c + " ";
	else if(!has_class(c)) css_class += c + " ";
	if(style) style->update(css_class);
}

void Component::remove_class(const std::string& c) {
	size_t i = css_class.find(" " + c + " ");
	if(i!=std::string::npos) css_class.erase(i, c.length()+1);
	if(css_class==" ") css_class = "";
	if(style) style->update(css_class);
}

void Component::remove_all_classes() {
	css_class="";
	if(style) style->update(css_class);
}


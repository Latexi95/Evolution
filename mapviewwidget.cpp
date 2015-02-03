#include "mapviewwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <iostream>

MapViewWidget::MapViewWidget(QWidget *parent) :
	QWidget(parent),
	mMap(0),
	mDrawing(true) {

}

MapViewWidget::~MapViewWidget() {
}

Map *MapViewWidget::map() const {
	return mMap;
}

void MapViewWidget::setMap(Map *map) {
	mMap = map;
}

void MapViewWidget::paintEvent(QPaintEvent *e) {
	if (!mMap) return;
	if (!mDrawing || mMap->noDraw()) return;
	QPainter painter(this);
	painter.drawImage(0,0, mCurrentImage);
}
bool MapViewWidget::drawing() const {
	return mDrawing;
}

void MapViewWidget::setDrawing(bool value) {
	mDrawing = value;
}

void MapViewWidget::setCurrentImage(QImage img) {
	mCurrentImage = img;
	repaint();
}






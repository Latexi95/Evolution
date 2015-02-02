#include "mapviewwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <iostream>

MapViewWidget::MapViewWidget(QWidget *parent) :
	QWidget(parent),
	mMap(0),
	mDrawFlags(0) {

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, static_cast<void (MapViewWidget::*)()>(&MapViewWidget::update));
	timer->start(100);
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
	QPainter painter(this);
	if (mDrawFlags == NoDraw) return;
	mMap->draw(&painter, e->rect().x(), e->rect().y(), e->rect().width(), e->rect().height(), mDrawFlags);
}
int MapViewWidget::drawFlags() const {
	return mDrawFlags;
}

void MapViewWidget::setDrawFlags(int drawFlags) {
	mDrawFlags = drawFlags;
}




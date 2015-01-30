#include "mapviewwidget.h"
#include <QPainter>
MapViewWidget::MapViewWidget(QWidget *parent) :
	QWidget(parent),
	mMap(0),
	mDrawFlags(0) {

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, static_cast<void (MapViewWidget::*)()>(&MapViewWidget::repaint));
	timer->start(50);
}

MapViewWidget::~MapViewWidget() {
}

Map *MapViewWidget::map() const {
	return mMap;
}

void MapViewWidget::setMap(Map *map) {
	mMap = map;
}

void MapViewWidget::paintEvent(QPaintEvent *) {
	if (!mMap) return;

	QPainter painter(this);
	mMap->draw(&painter, 0, 0, mDrawFlags);
}
int MapViewWidget::drawFlags() const {
	return mDrawFlags;
}

void MapViewWidget::setDrawFlags(int drawFlags) {
	mDrawFlags = drawFlags;
}




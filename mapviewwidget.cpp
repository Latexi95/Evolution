#include "mapviewwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <iostream>
#include <QMouseEvent>
#include <QAction>

MapViewWidget::MapViewWidget(QWidget *parent) :
	QWidget(parent),
	mMap(0),
	mDrawing(true),
	mZoomLevel(1) {
	mZoomIn = new QAction(this);
	mZoomIn->setShortcut(QKeySequence(Qt::Key_Plus));
	mZoomIn->setShortcutContext(Qt::WindowShortcut);
	mZoomOut = new QAction(this);
	mZoomOut->setShortcut(QKeySequence(Qt::Key_Minus));
	mZoomOut->setShortcutContext(Qt::WindowShortcut);
	this->addAction(mZoomIn);
	this->addAction(mZoomOut);

	connect(mZoomIn, &QAction::triggered, [&](){
		mZoomLevel++;
		mMapPos.rx() *= mZoomLevel;
		mMapPos.ry() *= mZoomLevel;
		mMapPos.rx() /= (mZoomLevel - 1);
		mMapPos.ry() /= (mZoomLevel - 1);
		if (mZoomLevel > 10) mZoomLevel = 10;
		repaint();
	});
	connect(mZoomOut, &QAction::triggered, [&](){
		mZoomLevel--;
		mMapPos.rx() *= mZoomLevel;
		mMapPos.ry() *= mZoomLevel;
		mMapPos.rx() /= (mZoomLevel + 1);
		mMapPos.ry() /= (mZoomLevel + 1);
		if (mZoomLevel == 0) mZoomLevel = 1;
		repaint();
	});
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
	if (!mDrawing || mCurrentImage.isNull()) return;
	QPainter painter(this);
	painter.drawImage(QRect(QPoint(mMapPos.x(),mMapPos.y()), mCurrentImage.size()*mZoomLevel), mCurrentImage, mCurrentImage.rect());
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

void MapViewWidget::mousePressEvent(QMouseEvent *e) {
	mMouseLastPosition = e->pos();
	if (e->button() == Qt::RightButton) {
		mDragging = true;
	}
	else if (e->button() == Qt::LeftButton) {
		QPoint mapPos = e->pos() - mMapPos;
		mapPos.rx() /= mZoomLevel;
		mapPos.ry() /= mZoomLevel;
		if (mapPos.x() < 0 || mapPos.y() < 0) return;
		if (mapPos.x() >= mMap->width() || mapPos.y() >= mMap->height()) return;
		emit mapPointClicked(mapPos);
	}
}

void MapViewWidget::mouseMoveEvent(QMouseEvent *e) {
	if (mDragging) {
		mMapPos -= mMouseLastPosition - e->pos();
	}
	mMouseLastPosition = e->pos();
	repaint();
}

void MapViewWidget::mouseReleaseEvent(QMouseEvent *e) {
	if (e->button() == Qt::RightButton) {
		mDragging = false;
	}
}







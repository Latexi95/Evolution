#ifndef MAPVIEWWIDGET_H
#define MAPVIEWWIDGET_H

#include <QWidget>
#include "map.h"
#include <QTimer>

class MapViewWidget : public QWidget {
		Q_OBJECT
	public:
		explicit MapViewWidget(QWidget *parent = 0);
		~MapViewWidget();

		Map *map() const;
		void setMap(Map *map);

		bool drawing() const;
		void setDrawing(bool value);

	signals:
		void mapPointClicked(QPoint mapPos);
	public slots:
		void setCurrentImage(QImage img);
	private:
		void paintEvent(QPaintEvent *);
		void mousePressEvent(QMouseEvent *e);
		void mouseMoveEvent(QMouseEvent *e);
		void mouseReleaseEvent(QMouseEvent *e);

		Map *mMap;
		bool mDrawing;
		QImage mCurrentImage;
		QPoint mMouseLastPosition;
		QPoint mMapPos;
		int mZoomLevel;
		QAction *mZoomIn;
		QAction *mZoomOut;
		bool mDragging;
};

#endif // MAPVIEWWIDGET_H

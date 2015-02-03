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

	public slots:
		void setCurrentImage(QImage img);
	private:
		void paintEvent(QPaintEvent *);


		Map *mMap;
		bool mDrawing;
		QImage mCurrentImage;
};

#endif // MAPVIEWWIDGET_H

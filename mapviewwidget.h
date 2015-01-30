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
		int drawFlags() const;
		void setDrawFlags(int drawFlags);

	signals:

	public slots:

	private:
		void paintEvent(QPaintEvent *);


		Map *mMap;
		int mDrawFlags;
};

#endif // MAPVIEWWIDGET_H

#ifndef WORKER_H
#define WORKER_H
#include <QRunnable>
#include <QTime>
#include "map.h"
#include <QThreadPool>

struct WorkResults {
	int mEntities;
	quint64 mTicks;
	quint64 mGeneration;
	int mTaskSize;
	double mTotalTime;
	double mExecutionTime;
};

Q_DECLARE_METATYPE(WorkResults)

class Worker : public QObject, public QRunnable {
		Q_OBJECT
	public:
		Worker(Map *map);
		~Worker();
		void run();
		void stop();
	signals:
		void finished();
		void workResults(WorkResults results);
		void drawFinished(QImage img);
	private:
		Map *mMap;
		QTime mLastUpdate;
		QThreadPool mThreadPool;
		int mDrawTimeout;
		volatile bool mRunning;
};

#endif // WORKER_H

#include "worker.h"
#include "entityupdatetask.h"
#include "action.h"
#include <chrono>

const int entitiesPerTask = 1500;
#define DRAW_TIMEOUT 70
Worker::Worker(Map *map) :
	mMap(map),
	mRunning(false){
	mDrawTimeout = DRAW_TIMEOUT;
	setAutoDelete(false);
}

Worker::~Worker() {
}

void Worker::run() {
	mRunning = true;
	mLastUpdate.start();
	WorkResults results;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime, execEndTime, totalEndTime;
	while (mRunning) {
		mMap->updateFoodLevels();
		QVector<Entity*> taskData;
		QVector<EntityUpdateTask*> tasks;
		quint64 generation = 0;
		startTime = std::chrono::high_resolution_clock::now();
		for (Entity *e : mMap->entities()) {
			if (e->generation() > generation) generation = e->generation();
			taskData.append(e);
			if (taskData.size() == entitiesPerTask) {
				EntityUpdateTask *task = new EntityUpdateTask(mMap, taskData);
				tasks.append(task);
	#ifndef NO_THREADS
				mThreadPool.start(task);
	#endif
				taskData.clear();
			}
		}


		if (!taskData.isEmpty()) {
			EntityUpdateTask *task = new EntityUpdateTask(mMap, taskData);
			tasks.append(task);
	#ifndef NO_THREADS
			mThreadPool.start(task);
	#endif
		}

		if (mLastUpdate.elapsed() > mDrawTimeout) {
			emit drawFinished(mMap->draw());
			mLastUpdate.restart();
		}

	#ifdef NO_THREADS
		for (EntityUpdateTask *task : tasks) task->run();
	#else
		mThreadPool.waitForDone();
	#endif
		execEndTime = std::chrono::high_resolution_clock::now();


		std::multimap<EntityProperty::ValueType, Action*> speedSortedActions;
		for (EntityUpdateTask *task : tasks) {
			for (Action *action : task->actions()) {
				if (action->shouldBeSpeedSorted())
					speedSortedActions.insert(std::pair<EntityProperty::ValueType, Action*>(std::max(action->speed().value(), action->entity()->energy().value()), action));
				else {
					EntityProperty result = action->exec(mMap);
					action->entity()->reportActionResult(result);
					delete action;
				}
			}
			delete task;
		}

		if (!speedSortedActions.empty()) {
			auto it = speedSortedActions.end();
			do {
				--it;
				EntityProperty result = it->second->exec(mMap);
				it->second->entity()->reportActionResult(result);
				delete it->second;
			} while (it != speedSortedActions.begin());
		}

		mMap->deletePass();
		if (mMap->entities().size() < 5000) {
			for (int i = 0; i < 30; i++) {
				mMap->createAndRandomPlaceEntity();
			}
		}
		totalEndTime = std::chrono::high_resolution_clock::now();

		if (mMap->tick() % 20000 == 0) {
			mMap->save("autosave_" + QString::number(mMap->tick() / 5000));
		}

		results.mEntities = mMap->entities().size();
		results.mGeneration = generation;
		results.mTaskSize = tasks.size();
		results.mTicks = mMap->tick();
		results.mExecutionTime = std::chrono::duration_cast<std::chrono::microseconds>(execEndTime - startTime).count();
		results.mTotalTime = std::chrono::duration_cast<std::chrono::microseconds>(totalEndTime - startTime).count();
		if (mMap->tick() % 5 == 0) {
			emit workResults(results);
		}
	}
	emit workResults(results);
	emit finished();
}

void Worker::stop() {
	mRunning = false;
}

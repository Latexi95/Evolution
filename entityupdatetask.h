#ifndef ENTITYUPDATETASK_H
#define ENTITYUPDATETASK_H

#include <QRunnable>
#include <QVector>
class Entity;
class Action;
class Map;
class EntityUpdateTask : public QRunnable {
	public:
		EntityUpdateTask(const Map *map, const QVector<Entity*> &entities);
		~EntityUpdateTask();
		void run();
		const QVector<Action*> &actions();
	private:
		const QVector<Entity*> mEntities;
		const Map *mMap;
		QVector<Action*> mActions;
};


#endif // ENTITYUPDATETASK_H

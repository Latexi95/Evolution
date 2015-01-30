#include "entityupdatetask.h"
#include "entity.h"
EntityUpdateTask::EntityUpdateTask(const Map *map, const QVector<Entity *> &entities) :
	mEntities(entities),
	mMap(map) {
	mActions.reserve(entities.size());
	setAutoDelete(false);
}

EntityUpdateTask::~EntityUpdateTask() {

}

void EntityUpdateTask::run() {
	for (Entity *entity : mEntities) {
		Action *a = entity->update(mMap);
		if (a) {
			mActions.append(a);
		}
	}
}

const QVector<Action *> &EntityUpdateTask::actions() {
	return mActions;
}


#include "action.h"
#include "map.h"
#include "entity.h"
Action::Action(Entity *entity, EntityProperty speed) :
	mEntity(entity),
	mSpeed(speed) {

}

Action::~Action() {

}

EntityProperty Action::speed() const {
	return mSpeed;
}



MoveAction::MoveAction(Entity *entity, EntityProperty speed, Direction direction) :
	Action(entity, speed),
	mDirection(direction) {
}

MoveAction::~MoveAction() {
}

EntityProperty MoveAction::exec(Map *map) const {
	mEntity->energy() -= mSpeed / 5 + 7;
	if (map->move(mEntity, mEntity->position().targetLocation(mDirection, 1))) {
		return EntityProperty::max();
	}
	return EntityProperty::min();
}

Direction MoveAction::direction() const {
	return mDirection;
}


AttackAction::AttackAction(Entity *entity, EntityProperty speed, Direction direction, EntityProperty power) :
	Action(entity, speed),
	mDirection(direction),
	mPower(power) {

}

AttackAction::~AttackAction() {
}

EntityProperty AttackAction::exec(Map *map) const {
	Position targetPos = mEntity->position().targetLocation(mDirection, 1);
	if (map->isPositionOnMap(targetPos)) {
		Tile &tile = map->tile(targetPos);

		mEntity->energy() -= mSpeed / 2 + 2;
		if (!tile.mEntity) {//Nothing in target tile
			return EntityProperty::min();
		}


		EntityProperty usedPower = mEntity->energy().take(mPower);

		EntityProperty damageDealt = tile.mEntity->health().take((usedPower).greater(1));
		return damageDealt;
	}
	else {
		return EntityProperty::min();
	}

}

Direction AttackAction::direction() const {
	return mDirection;
}

EntityProperty AttackAction::power() const {
	return mPower;
}

EatAction::EatAction(Entity *entity, EntityProperty speed, FoodType type) :
	Action(entity, speed),
	mFoodType(type) {

}

EatAction::~EatAction() {
}

EntityProperty EatAction::exec(Map *map) const {
	Tile &tile = map->tile(mEntity->position());
	EntityProperty usedEnergy = mEntity->energy().take(mSpeed / 2 + 5) + 1;
	EntityProperty eaten = tile.mFoodLevels[(int)mFoodType].take(usedEnergy * 50);
	mEntity->energy() += eaten * 2 / 30;
	return eaten;
}

FoodType EatAction::foodType() const {
	return mFoodType;
}



HealAction::HealAction(Entity *entity, EntityProperty speed) :
	Action(entity, speed) {
}

HealAction::~HealAction() {
}

EntityProperty HealAction::exec(Map *) const {
	EntityProperty used = mEntity->energy().take(mSpeed / 2 + 5);
	mEntity->health() += used / 2;
	return used;
}


ReproduceAction::ReproduceAction(Entity *entity, EntityProperty speed) :
	Action(entity, speed) {
}

ReproduceAction::~ReproduceAction() {
}

EntityProperty ReproduceAction::exec(Map *map) const {
	mEntity->energy() -= 20;
	if (mEntity->energy() > mSpeed * 3) {
		Entity *child = map->createNewEntity(mEntity);
		if (!child) {
			return EntityProperty::min();
		}

		child->energy() = mEntity->energy().take(mSpeed * 3) / 2;

		return child->energy();
	}
	return EntityProperty::min();
}

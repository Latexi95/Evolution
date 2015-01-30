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
	if (map->move(mEntity, mEntity->position().targetLocation(mDirection, 1))) {
		mEntity->energy() -= mSpeed;
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


		if (!tile.mEntity) {//Nothing in target tile
			return EntityProperty::min();
		}

		mEntity->energy() -= mSpeed;
		EntityProperty usedPower = mEntity->energy().take(mPower);

		EntityProperty damageDealt = tile.mEntity->health().take((usedPower / 10).greater(1));
		return damageDealt + 1;
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
	EntityProperty usedSpeed = mEntity->energy().take(mSpeed) + 1;
	EntityProperty eaten = tile.mFoodLevels[(int)mFoodType].take((usedSpeed + usedSpeed / 2).sqrt()) * 2;
	mEntity->energy() += eaten;
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

EntityProperty HealAction::exec(Map *map) const {
	EntityProperty used = mEntity->energy().take(mSpeed);
	mEntity->health() += used;
	return used;
}

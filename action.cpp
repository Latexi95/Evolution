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
	int waterGenLevel = map->tile(mEntity->position()).mWaterGenLevel;
	waterGenLevel *= waterGenLevel;
	mEntity->energy() -= mSpeed / 5 + 6 + waterGenLevel / 1024;
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
		int waterGenLevel = map->tile(mEntity->position()).mWaterGenLevel;
		waterGenLevel *= waterGenLevel;
		mEntity->energy() -= mSpeed / 4 + 1 + waterGenLevel / 2048;
		if (!tile.mEntity) {//Nothing in target tile
			return EntityProperty::min();
		}


		EntityProperty usedPower = mEntity->energy().take(mPower / 2 + 10);

		EntityProperty damageDealt = tile.mEntity->health().take((usedPower * 8).greater(2));
		return damageDealt;
	}
	return EntityProperty::min();

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
	EntityProperty tryEat = usedEnergy.sqrt() * ((tile.mFoodLevels[(int)mFoodType] / (mEntity->foodLevelAdaption() + 20)).square() / 25 + (tile.mFoodLevels[(int)mFoodType] / 5 * (mEntity->foodLevelAdaption() / 2 + 3)/ 5));
	EntityProperty eaten = tile.mFoodLevels[(int)mFoodType].take(tryEat) * 2;
	mEntity->energy() += eaten.sqrt();
	if (mEntity->energy() > mEntity->maxEnergy()) {
		mEntity->energy() = mEntity->maxEnergy();
	}
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
	if (mEntity->health() > mEntity->maxHealth()) {
		mEntity->health() = mEntity->maxHealth();
	}
	return used;
}


ReproduceAction::ReproduceAction(Entity *entity, EntityProperty speed) :
	Action(entity, speed) {
}

ReproduceAction::~ReproduceAction() {
}

EntityProperty ReproduceAction::exec(Map *map) const {
	mEntity->energy() -= 20;
	if (mEntity->energy() > mSpeed * 4 && mEntity->hydration() > mSpeed * 3) {
		Entity *child = map->createAndPlaceEntity(mEntity);
		if (!child) {
			return EntityProperty::min();
		}

		child->energy() = mEntity->energy().take(mSpeed * 3) / 2;
		child->hydration() = mEntity->hydration().take(mSpeed * 3);
		child->health() = mEntity->maxHealth() / 7;

		return child->energy();
	}
	return EntityProperty::min();
}


CommunicateAction::CommunicateAction(Entity *entity, EntityProperty speed, Entity *target, EntityProperty::ValueType id, EntityProperty value) :
	Action(entity, speed),
	mTarget(target),
	mId(id),
	mValue(value) {
}

CommunicateAction::~CommunicateAction() {
}

EntityProperty CommunicateAction::exec(Map *) const {
	mEntity->energy() -= mSpeed / 3;
	EntityProperty oldValue = mTarget->loadStore(mId);
	mTarget->saveStore(mId, mValue);
	return oldValue;
}


DrinkAction::DrinkAction(Entity *entity, EntityProperty speed) :
	Action(entity, speed) {

}

DrinkAction::~DrinkAction() {

}

EntityProperty DrinkAction::exec(Map *map) const {
	Tile &t = map->tile(entity()->position());
	EntityProperty energyUsed = entity()->energy().take(mEntity->drinkEnergyCost(mSpeed));
	EntityProperty water = (int)t.mWaterLevel.take((energyUsed + 10) * 5).value() * 60 / (20 + mEntity->hydrationAdaption().value());
	entity()->hydration() += water;
	return water;
}

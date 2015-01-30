#ifndef ACTION_H
#define ACTION_H
#include "enums.h"
#include "entityproperty.h"

class Entity;
class Map;

class Action {
	public:
		enum Type {
			Move,
			Attack,
			Eat,
			Heal
		};

		Action(Entity *entity, EntityProperty speed);
		virtual ~Action();
		virtual Type type() const = 0;
		virtual EntityProperty exec(Map *map) const = 0;
		EntityProperty speed() const;
		Entity *entity() const { return mEntity; }
	protected:
		Entity *mEntity;
		EntityProperty mSpeed;
};

class MoveAction : public Action {
	public:
		MoveAction(Entity *entity, EntityProperty speed, Direction direction);
		~MoveAction();
		Type type() const { return Move; }
		EntityProperty exec(Map *map) const;
		Direction direction() const;
	private:
		Direction mDirection;
};

class AttackAction : public Action {
	public:
		AttackAction(Entity *entity, EntityProperty speed, Direction direction, EntityProperty power);
		~AttackAction();
		Type type() const { return Attack; }
		EntityProperty exec(Map *map) const;
		Direction direction() const;
		EntityProperty power() const;
	private:
		Direction mDirection;
		EntityProperty mPower;
};

class EatAction : public Action {
	public:
		EatAction(Entity *entity, EntityProperty speed, FoodType type);
		~EatAction();
		Type type() const { return Eat; }
		EntityProperty exec(Map *map) const;
		FoodType foodType() const;

	private:
		FoodType mFoodType;
};

class HealAction : public Action {
	public:
		HealAction(Entity *entity, EntityProperty speed);
		~HealAction();
		Type type() const { return Heal; }
		EntityProperty exec(Map *map) const;
};

#endif // ACTION_H

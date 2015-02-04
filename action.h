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
			Heal,
			Reproduce,
			Communicate,
			Drink
		};

		Action(Entity *entity, EntityProperty speed);
		virtual ~Action();
		virtual Type type() const = 0;
		virtual EntityProperty exec(Map *map) const = 0;
		virtual bool shouldBeSpeedSorted() const = 0;
		virtual bool canBeThreaded() const = 0;
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
		bool shouldBeSpeedSorted() const { return true; }
		bool canBeThreaded() const { return false; }
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
		bool shouldBeSpeedSorted() const { return true; }
		bool canBeThreaded() const { return false; }
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
		bool shouldBeSpeedSorted() const { return false; }
		bool canBeThreaded() const { return true; }
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
		bool shouldBeSpeedSorted() const { return false; }
		bool canBeThreaded() const { return true; }
		EntityProperty exec(Map *map) const;
};

class ReproduceAction : public Action {
	public:
		ReproduceAction(Entity *entity, EntityProperty speed);
		~ReproduceAction();
		Type type() const { return Reproduce; }
		bool shouldBeSpeedSorted() const { return false; }
		bool canBeThreaded() const { return false; }
		EntityProperty exec(Map *map) const;
};

class CommunicateAction : public Action {
	public:
		CommunicateAction(Entity *entity, EntityProperty speed, Entity *target, EntityProperty::ValueType id, EntityProperty value);
		~CommunicateAction();
		Type type() const { return Communicate; }
		bool shouldBeSpeedSorted() const { return false; }
		bool canBeThreaded() const { return false; }
		EntityProperty exec(Map *) const;
	private:
		Entity *mTarget;
		EntityProperty::ValueType mId;
		EntityProperty mValue;
};

class DrinkAction : public Action {
	public:
		DrinkAction(Entity *entity, EntityProperty speed);
		~DrinkAction();
		bool shouldBeSpeedSorted() const { return false; }
		bool canBeThreaded() const { return true; }
		Type type() const { return Drink; }
		EntityProperty exec(Map *map) const;
};

#endif // ACTION_H

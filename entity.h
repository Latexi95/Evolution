#ifndef ENTITY_H
#define ENTITY_H
#include <QString>
#include "entityproperty.h"
#include "position.h"
#include "enums.h"
#include <QHash>
#include <QVector>

class Action;
class Map;

enum class OpCode : quint8 {
	Literal,
	LiteralPrimary,
	LiteralSecondary,
	Copy,
	CopyResultToPrimary,
	CopyResultToSecondary,
	Load,
	Equal,
	Greater,
	Add,
	Substract,
	And,
	Or,
	Not,
	True,
	SetSpeed,
	SetPower,
	GetSpeed,
	GetPower,
	GetHealt,
	GetMaxHealt,
	Eat,
	Move,
	Attack,
	Heal,
	ResetTargetMarker,
	MoveTargetMarker,
	IsTargetMarkerOnMap,
	GetFoodLevel,
	ContainsEntity,

	EntityCheckSum,
	SelfCheckSum,

	CheckEntityHealth,
	CheckEntitySpeed,

	ConditionalJump,
	Jump,


	MaxOpCode
};

struct Instruction {
	Instruction() : mOpCode(OpCode::MaxOpCode), mParam(0) {}
	Instruction(OpCode code, EntityProperty::ValueType param) : mOpCode(code), mParam(param) {}
	OpCode mOpCode;
	EntityProperty::ValueType mParam;
};


class Entity {
	public:
		Entity();
		~Entity();

		Position position() const;
		void setPosition(const Position &position);

		Action *update(const Map *map);

		EntityProperty &health();
		EntityProperty &energy();
		EntityProperty &speed();

		void reportActionResult(EntityProperty success);

		EntityProperty byteCodeCheckSum();

		const QVector<Instruction> &byteCode() const;
		void setByteCode(const QVector<Instruction> &byteCode);

		bool deletePass();


		QString byteCodeAsString() const;

		quint64 lifeTime() const;

	private:
		const Instruction &instruction();
		void nextInstruction();
		Action *exec(const Map *map, const int maxInstruction, int &instructionCounter);
		Action *execInstruction(const Map *map, const Instruction &ins);
		static Direction directionFromParam(EntityProperty::ValueType param);
		static FoodType foodTypeFromParam(EntityProperty::ValueType param);
		Position targetMarkerPosition() const;

		EntityProperty mHealth;
		EntityProperty mMaxHealth;
		EntityProperty mEnergy;
		EntityProperty mSpeed;
		EntityProperty mPower;
		Position mPosition;
		Position mTargetMarker;
		EntityProperty mResultRegister;
		EntityProperty mPrimaryRegister;
		EntityProperty mSecondaryRegister;
		quint64 mLifeTime;

		QHash<EntityProperty::ValueType, EntityProperty> mData;

		QVector<Instruction> mByteCode;
		QVector<Instruction>::ConstIterator mExectionPoint;
};

#endif // ENTITY_H

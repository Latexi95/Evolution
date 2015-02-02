#ifndef ENTITY_H
#define ENTITY_H
#include <QString>
#include "entityproperty.h"
#include "position.h"
#include "enums.h"
#include <QHash>
#include <QVector>
#include <QMap>

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
	GetEnergy,
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

	Reproduce,

	LoadEntityStore,
	CopyEntityStore,


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

		quint64 generation() const;
		void setGeneration(quint64 gen);

		void save(QDataStream &stream, int format) const;

		void load(QDataStream &stream, int format);


		EntityProperty loadStore(EntityProperty::ValueType id) const;
		void saveStore(EntityProperty::ValueType id, const EntityProperty &val);
	private:
		const Instruction &instruction() const;
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

		QMap<EntityProperty::ValueType, EntityProperty> mData;

		QVector<Instruction> mByteCode;
		int mExecutionPoint;

		quint64 mGeneration;
};

inline const Instruction &Entity::instruction() const {
	return mByteCode[mExecutionPoint];
}

inline void Entity::nextInstruction() {
	++mExecutionPoint;
	if (mExecutionPoint == mByteCode.size()) {
		mExecutionPoint = 0;
	}
}

inline Position Entity::targetMarkerPosition() const {
	return mPosition + mTargetMarker;
}

QDataStream &operator << (QDataStream &out, const Instruction &ins);
QDataStream &operator >> (QDataStream &in, Instruction &ins);
#endif // ENTITY_H

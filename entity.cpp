#include "entity.h"
#include "map.h"
#include "action.h"
#include <cassert>
#include <QDataStream>

Entity::Entity() :
	mHealth(100),
	mMaxHealth(150),
	mEnergy(20),
	mSpeed(10),
	mPower(10),
	mHydration(100),
	mLifeTime(0),
	mGeneration(1),
	mExecutionEnergyUsageCounter(0) {

}

Entity::~Entity() {

}

Position Entity::position() const {
	return mPosition;
}

void Entity::setPosition(const Position &position) {
	mPosition = position;
}

Action *Entity::update(const Map *map) {
	mLifeTime++;

	mHydration -= 32 + map->tile(position()).mHeat / 8;
	if (mHydration == 0) {
		mEnergy -= 20;
		mHealth -= 1;
	}
	else if (mHydration < 50) {
		mEnergy -= 5;
	}

	int instructionCounter;
	const int maxInstructions = 1000;
	Action *action = exec(map, maxInstructions, instructionCounter);
	mExecutionEnergyUsageCounter += mByteCode.size() + instructionCounter;
	while (mExecutionEnergyUsageCounter > 500) {
		mEnergy -= 1;
		mExecutionEnergyUsageCounter -= 500;
	}
	mEnergy -= 1;
	return action;
}

EntityProperty &Entity::health() {
	return mHealth;
}

EntityProperty &Entity::energy() {
	return mEnergy;
}

EntityProperty &Entity::speed() {
	return mSpeed;
}

EntityProperty &Entity::hydration() {
	return mHydration;
}

void Entity::reportActionResult(EntityProperty success) {
	mResultRegister = success;
}

EntityProperty Entity::byteCodeCheckSum() {
	EntityProperty::ValueType checkSum = 0;
	for (const Instruction &ins : mByteCode) {
		checkSum ^= (EntityProperty::ValueType)ins.mOpCode;
		checkSum = (checkSum << 1) | (checkSum >> (sizeof(checkSum)*CHAR_BIT-1));
	}
	EntityProperty ret(checkSum);
	if (ret == EntityProperty::min()) ret = EntityProperty::max();
	return ret;
}

Action *Entity::exec(const Map *map, const int maxInstruction, int &instructionCounter) {
	instructionCounter = 0;
	Action *action = execInstruction(map, instruction());
	nextInstruction();
	if (action != nullptr) return action;
	instructionCounter++;
	for (;instructionCounter < maxInstruction; instructionCounter++) {
		action = execInstruction(map, instruction());
		nextInstruction();
		if (action != nullptr) return action;
	}
	return 0;
}

Action* Entity::execInstruction(const Map *map, const Instruction &ins) {
	switch (ins.mOpCode) {
		case OpCode::Literal:
			mResultRegister = ins.mParam;
			break;
		case OpCode::LiteralPrimary:
			mPrimaryRegister = ins.mParam;
			break;
		case OpCode::LiteralSecondary:
			mSecondaryRegister = ins.mParam;
			break;
		case OpCode::Copy:
			mData[ins.mParam] = mResultRegister;
			break;
		case OpCode::CopyResultToPrimary:
			mPrimaryRegister = mResultRegister;
			break;
		case OpCode::CopyResultToSecondary:
			mSecondaryRegister = mResultRegister;
			break;
		case OpCode::Load:
			mResultRegister = mData.value(ins.mParam);
			break;
		case OpCode::Equal:
			mResultRegister = mPrimaryRegister.equal(mSecondaryRegister);
			break;
		case OpCode::Greater:
			if (mPrimaryRegister > mSecondaryRegister) {
				mResultRegister = EntityProperty::max();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::Add:
			mResultRegister = mPrimaryRegister + mSecondaryRegister;
			break;
		case OpCode::Substract:
			mResultRegister = mPrimaryRegister - mSecondaryRegister;
			break;
		case OpCode::And:
			mResultRegister = mPrimaryRegister.logicalAnd(mSecondaryRegister);
			break;
		case OpCode::Or:
			mResultRegister = mPrimaryRegister.logicalOr(mSecondaryRegister);
			break;
		case OpCode::Not:
			mResultRegister = mPrimaryRegister.logicalNot();
			break;
		case OpCode::True:
			mResultRegister = mPrimaryRegister.binarized();
			break;
		case OpCode::SetSpeed:
			mSpeed = ins.mParam;
			mResultRegister = mSpeed;
			break;
		case OpCode::SetPower:
			mPower = ins.mParam;
			mResultRegister = mPower;
			break;
		case OpCode::GetSpeed:
			mResultRegister = mSpeed;
			break;
		case OpCode::GetPower:
			mResultRegister = mPower;
			break;
		case OpCode::GetHealt:
			mResultRegister = mHealth;
			break;
		case OpCode::GetMaxHealt:
			mResultRegister = mMaxHealth;
			break;
		case OpCode::GetEnergy:
			mResultRegister = mEnergy;
			break;
		case OpCode::Eat:
			return new EatAction(this, mSpeed, foodTypeFromParam(ins.mParam));

		case OpCode::Move:
			return new MoveAction(this, mSpeed, directionFromParam(ins.mParam));
		case OpCode::Attack:
			return new AttackAction(this, mSpeed, directionFromParam(ins.mParam), mPower);
		case OpCode::Heal:
			return new HealAction(this, mSpeed);
		case OpCode::ResetTargetMarker:
			mTargetMarker = Position();
			break;
		case OpCode::MoveTargetMarker:
			mTargetMarker = mTargetMarker.targetLocation(directionFromParam(ins.mParam), 1);
			break;
		case OpCode::IsTargetMarkerOnMap:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				mResultRegister = EntityProperty::max();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::GetFoodLevel:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				mResultRegister = map->tile(targetMarkerPosition()).mFoodLevels[(int)foodTypeFromParam(ins.mParam)];
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::ContainsEntity: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				mResultRegister = EntityProperty::max();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::EntityCheckSum: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				mResultRegister = entity->byteCodeCheckSum();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::SelfCheckSum:
			mResultRegister = byteCodeCheckSum();
			break;
		case OpCode::CheckEntityHealth: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				mResultRegister = entity->health();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::CheckEntitySpeed: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				mResultRegister = entity->speed();
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::ConditionalJump:
			if (!mPrimaryRegister.isMin()) {
				int jump = ins.mParam % mByteCode.size();
				for (int i = 0; i < jump - 1; i++) {
					nextInstruction();
				}
			}
			break;
		case OpCode::Jump: {
			int jump = ins.mParam % mByteCode.size();
			for (int i = 0; i < jump - 1; i++) {
				nextInstruction();
			}
			break;
		}
		case OpCode::Reproduce:
			return new ReproduceAction(this, mSpeed);

		case OpCode::LoadEntityStore: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				mResultRegister = entity->loadStore(ins.mParam);
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::CopyEntityStore: {
			Entity *entity = map->entity(targetMarkerPosition());
			if (entity) {
				return new CommunicateAction(this, mSpeed, entity, ins.mParam, mPrimaryRegister);
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		}
		case OpCode::Drink: {
			return new DrinkAction(this, mSpeed);
		}
		case OpCode::CheckHydrationLevel: {
			mResultRegister = mHydration;
			break;
		}
		case OpCode::CheckWaterLevel:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				mResultRegister = map->tile(targetMarkerPosition()).mWaterLevel;
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::CheckHeatLevel:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				mResultRegister = map->tile(targetMarkerPosition()).mHeat;
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::MaxOpCode:
			assert("Max op code" && 0);
	}
	return 0;
}

Direction Entity::directionFromParam(EntityProperty::ValueType param) {
	param &= 0x3;
	switch (param) {
		case 1:
			return Right;
		case 2:
			return Up;
		case 3:
			return Down;
		default:
			return Left;
	}
}

FoodType Entity::foodTypeFromParam(EntityProperty::ValueType param) {
	param &= 1;
	if (param) return FoodType::M;
	return FoodType::V;
}

quint64 Entity::lifeTime() const {
	return mLifeTime;
}

quint64 Entity::generation() const {
	return mGeneration;
}

void Entity::setGeneration(quint64 gen) {
	mGeneration = gen;
}

void Entity::save(QDataStream &stream, int format) const {
	stream << mHealth;
	stream << mMaxHealth;
	stream << mEnergy;
	stream << mSpeed;
	stream << mPosition;
	stream << mTargetMarker;
	stream << mResultRegister;
	stream << mPrimaryRegister;
	stream << mSecondaryRegister;
	stream << mLifeTime;
	stream << mData;
	stream << mByteCode;
	stream << mExecutionPoint;
	stream << mGeneration;
}

void Entity::load(QDataStream &stream, int format) {
	stream >> mHealth;
	stream >> mMaxHealth;
	stream >> mEnergy;
	stream >> mSpeed;
	stream >> mPosition;
	stream >> mTargetMarker;
	stream >> mResultRegister;
	stream >> mPrimaryRegister;
	stream >> mSecondaryRegister;
	stream >> mLifeTime;
	stream >> mData;
	stream >> mByteCode;
	stream >> mExecutionPoint;
	stream >> mGeneration;
}

EntityProperty Entity::loadStore(EntityProperty::ValueType id) const {
	return mData.value(id);
}

void Entity::saveStore(EntityProperty::ValueType id, const EntityProperty &val) {
	mData[id] = val;
}

const QVector<Instruction> &Entity::byteCode() const {
	return mByteCode;
}

void Entity::setByteCode(const QVector<Instruction> &byteCode) {
	mByteCode = byteCode;
	mExecutionPoint = 0;
}





bool Entity::deletePass() {

	if (mEnergy == EntityProperty::min()) {//No Energy
		mHealth -= 3;
	}
	if (mHealth == EntityProperty::min()) {
		return true;
	}
	return false;
}

QString Entity::byteCodeAsString() const {
	QString text;
	for (const Instruction &ins : mByteCode) {
		switch (ins.mOpCode) {
			case OpCode::Literal:
				text += "Literal: " + QString::number(ins.mParam);
				break;
			case OpCode::LiteralPrimary:
				text += "LiteralPrimary: " + QString::number(ins.mParam);
				break;
			case OpCode::LiteralSecondary:
				text += "Literal: " + QString::number(ins.mParam);
				break;
			case OpCode::Copy:
				text += "Copy: " + QString::number(ins.mParam);
				break;
			case OpCode::CopyResultToPrimary:
				text += "CopyResToPri";
				break;
			case OpCode::CopyResultToSecondary:
				text += "CopyResToSec";
				break;
			case OpCode::Load:
				text += "Load: " + QString::number(ins.mParam);
				break;
			case OpCode::Equal:
				text += "Eq";
				break;
			case OpCode::Greater:
				text += "Greater";
				break;
			case OpCode::Add:
				text += "Add";
				break;
			case OpCode::Substract:
				text += "Sub";
				break;
			case OpCode::And:
				text += "And";
				break;
			case OpCode::Or:
				text += "Or";
				break;
			case OpCode::Not:
				text += "Not";
				break;
			case OpCode::True:
				text += "True";
				break;
			case OpCode::SetSpeed:
				text += "SetSpeed: " + QString::number(ins.mParam);
				break;
			case OpCode::SetPower:
				text += "SetPower: " + QString::number(ins.mParam);
				break;
			case OpCode::GetSpeed:
				text += "GetSpeed";
				break;
			case OpCode::GetPower:
				text += "GetPower";
				break;
			case OpCode::GetHealt:
				text += "GetHealth";
				break;
			case OpCode::GetMaxHealt:
				text += "GetMaxHealt";
				break;
			case OpCode::GetEnergy:
				text += "GetEnergy";
				break;
			case OpCode::Eat: {
				text += QStringLiteral("Eat: ") + (foodTypeFromParam(ins.mParam) == FoodType::M ? "M" : "V");
				break;
			}

			case OpCode::Move: {
				text += QStringLiteral("Move: ") + directionToString(directionFromParam(ins.mParam));
				break;
			}
			case OpCode::Attack: {
				text += QStringLiteral("Attack: ") + directionToString(directionFromParam(ins.mParam));
				break;
			}
			case OpCode::Heal:
				text += "Heal";
				break;
			case OpCode::ResetTargetMarker:
				text += "ResetTargetMarker";
				break;
			case OpCode::MoveTargetMarker:
				text += QStringLiteral("MoveTargetMarker: ") + directionToString(directionFromParam(ins.mParam));
				break;
			case OpCode::IsTargetMarkerOnMap:
				text += "IsTargetMarkerOnMap";
				break;
			case OpCode::GetFoodLevel:
				text += QStringLiteral("GetFoodLevel: ") + (foodTypeFromParam(ins.mParam) == FoodType::M ? "M" : "V");
				break;
			case OpCode::ContainsEntity:
				text += "ContainsEntity";
				break;

			case OpCode::EntityCheckSum:
				text += "EntityCheckSum";
				break;
			case OpCode::SelfCheckSum:
				text += "SelfCheckSum";
				break;
			case OpCode::CheckEntityHealth:
				text += "CheckEntityHealth";
				break;
			case OpCode::CheckEntitySpeed:
				text += "CheckEntitySpeed";
				break;
			case OpCode::ConditionalJump:
				text += "CondJump: " + QString::number(ins.mParam);
				break;
			case OpCode::Jump: {
				text += "Jump: " + QString::number(ins.mParam);
				break;
			}
			case OpCode::Reproduce:
				text += "Reproduce";
				break;
			case OpCode::LoadEntityStore:
				text += "LoadEntityStore: " + QString::number(ins.mParam);;
				break;
			case OpCode::CopyEntityStore:
				text += "CopyEntityStore: " + QString::number(ins.mParam);;
				break;
			case OpCode::Drink: {
				text += "Drink";
				break;
			}
			case OpCode::CheckHydrationLevel: {
				text += "CheckHydrationLevel";
				break;
			}
			case OpCode::CheckWaterLevel:
				text += "CheckWaterLevel";
				break;
			case OpCode::CheckHeatLevel:
				text += "CheckHeatLevel";
				break;
			default:
				break;
		}
		text += '\n';
	}
	return text;
}


QDataStream &operator <<(QDataStream &out, const Instruction &ins) {
	out << (int)ins.mOpCode;
	out << ins.mParam;
	return out;
}


QDataStream &operator >>(QDataStream &in, Instruction &ins) {
	int opCode;
	in >> opCode >> ins.mParam;
	ins.mOpCode = (OpCode)opCode;
	return in;
}

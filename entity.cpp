#include "entity.h"
#include "map.h"
#include "action.h"
#include <cassert>

Entity::Entity() :
	mHealth(100),
	mMaxHealth(150),
	mEnergy(1000),
	mSpeed(10),
	mPower(10),
	mLifeTime(0) {

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
	int instructionCounter;
	const int maxInstructions = 1000;
	Action *action = exec(map, maxInstructions, instructionCounter);
	mEnergy -= instructionCounter / 100 + 1;
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

const Instruction &Entity::instruction() {
	return *mExectionPoint;
}

void Entity::nextInstruction() {
	++mExectionPoint;
	if (mExectionPoint == mByteCode.end()) {
		mExectionPoint = mByteCode.begin();
	}
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
		case OpCode::ContainsEntity:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				if (map->tile(targetMarkerPosition()).mEntity) {
					mResultRegister = EntityProperty::max();
				}
				else {
					mResultRegister = EntityProperty::min();
				}
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;

		case OpCode::EntityCheckSum:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				if (map->tile(targetMarkerPosition()).mEntity) {
					mResultRegister = map->tile(targetMarkerPosition()).mEntity->byteCodeCheckSum();
				}
				else {
					mResultRegister = EntityProperty::min();
				}
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::SelfCheckSum:
			mResultRegister = byteCodeCheckSum();
			break;
		case OpCode::CheckEntityHealth:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				if (map->tile(targetMarkerPosition()).mEntity) {
					mResultRegister = map->tile(targetMarkerPosition()).mEntity->health();
				}
				else {
					mResultRegister = EntityProperty::min();
				}
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
		case OpCode::CheckEntitySpeed:
			if (map->isPositionOnMap(targetMarkerPosition())) {
				if (map->tile(targetMarkerPosition()).mEntity) {
					mResultRegister = map->tile(targetMarkerPosition()).mEntity->speed();
				}
				else {
					mResultRegister = EntityProperty::min();
				}
			}
			else {
				mResultRegister = EntityProperty::min();
			}
			break;
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

Position Entity::targetMarkerPosition() const {
	return mPosition + mTargetMarker;
}
quint64 Entity::lifeTime() const {
	return mLifeTime;
}

const QVector<Instruction> &Entity::byteCode() const {
	return mByteCode;
}

void Entity::setByteCode(const QVector<Instruction> &byteCode) {
	mByteCode = byteCode;
	mExectionPoint = mByteCode.begin();
}





bool Entity::deletePass() {
	if (mEnergy == EntityProperty::min()) {//No Energy
		mHealth -= 1;
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
			default:
				break;
		}
		text += '\n';
	}
	return text;
}

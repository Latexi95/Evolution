#include "map.h"
#include "entity.h"
#include <QPainter>
Map::Map(int width, int height) :
	mWidth(width),
	mHeight(height),
	mTiles(width * height),
	mRandomGenerator(std::random_device()()){

	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Tile &t = tile(Position(x, y));
			t.mWaterLevel = 1000;
			t.mFoodLevels[(int)FoodType::V] = 1000;
		}
	}
}

Map::~Map() {

}

Tile &Map::tile(Position position) {
	return mTiles[position.x + mWidth * position.y];
}

const Tile &Map::tile(Position position) const {
	return mTiles[position.x + mWidth * position.y];
}

bool Map::isMovableLocation(Position target) const {
	if (!isPositionOnMap(target)) return false;
	return tile(target).mEntity == 0;
}

bool Map::move(Entity *entity, Position target) {
	if (!isMovableLocation(target)) return false;

	tile(entity->position()).mEntity = 0;
	tile(target).mEntity = entity;
	entity->setPosition(target);
	return true;
}

Position Map::findValidLocation(Position nearPos, int maxRange) {
	if (isMovableLocation(nearPos)) return nearPos;

	int level = 1;
	Position curPos = nearPos;
	int dir = 1;
	while (level <= maxRange) {
		for (int i = 0; i < level; i++) {
			curPos.x += dir;
			if (isMovableLocation(curPos)) return curPos;
		}
		for (int i = 0; i < level; i++) {
			curPos.y += dir;
			if (isMovableLocation(curPos)) return curPos;
		}
		level += 1;
		dir = -dir;
	}
	return Position::errorValue();
}

bool Map::isPositionOnMap(Position position) const {
	return position.x > 0 && position.x < mWidth && position.y > 0 && position.y < mHeight;
}

bool Map::addEntity(Entity *entity, Position pos) {
	if (isMovableLocation(pos)) {
		tile(pos).mEntity = entity;
		entity->setPosition(pos);
		mEntities.append(entity);
		return true;
	}
	return false;
}

void Map::removeEntity(Entity *entity) {
	tile(entity->position()).mFoodLevels[(int)FoodType::M] += 100;
	tile(entity->position()).mEntity = 0;
	mEntities.removeOne(entity);
}

int normalizeValue(EntityProperty property) {
	return property.sqrt().value();
}

void Map::draw(QPainter *p, int px, int py, int flags) {
	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {

			const Tile &t = tile(Position(x, y));
			int entityColor = 0;
			if (t.mEntity && (flags & NoEntities) != NoEntities) {
				entityColor = 127 + normalizeValue(t.mEntity->energy()) / 2;
			}
			int foodLevelVColor = 0;
			int foodLevelMColor = 0;
			if ((flags & NoMFoodLevels) != NoMFoodLevels) {
				foodLevelMColor = normalizeValue(t.foodLevel(FoodType::M));
			}
			if ((flags & NoVFoodLevels) != NoVFoodLevels) {
				foodLevelVColor = normalizeValue(t.foodLevel(FoodType::V));
			}
			p->setPen(QColor(
						  entityColor,
						  foodLevelVColor,
						  foodLevelMColor));
			p->drawPoint(px + x, py + y);
		}
	}

}

void Map::updateFoodLevels() {
	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Tile &t = tile(Position(x, y));
			if (!t.mEntity) t.mFoodLevels[(int)FoodType::V] += 15;
			t.mFoodLevels[(int)FoodType::M] -= 1;
		}
	}
}

const QList<Entity*> &Map::entities() const {
	return mEntities;
}

void Map::deletePass() {
	for (QList<Entity*>::Iterator i = mEntities.begin(); i != mEntities.end();) {
		if ((*i)->deletePass()) {
			tile((*i)->position()).mFoodLevels[(int)FoodType::M] += 1000;
			tile((*i)->position()).mEntity = 0;
			i = mEntities.erase(i);
		}
		else {
			++i;
		}
	}
}


void Map::randomFillMapWithEntities(int promil) {
	qsrand(12314);

	QVector<Instruction> byteCode;
	byteCode.append(Instruction(OpCode::Eat, 0));
	byteCode.append(Instruction(OpCode::ResetTargetMarker, 0));
	byteCode.append(Instruction(OpCode::MoveTargetMarker, 0));
	byteCode.append(Instruction(OpCode::GetFoodLevel, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::LiteralSecondary, 4000));
	byteCode.append(Instruction(OpCode::Greater, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 2));
	byteCode.append(Instruction(OpCode::Move, 2));
	byteCode.append(Instruction(OpCode::Jump, 1));
	byteCode.append(Instruction(OpCode::Move, 0));

	std::uniform_int_distribution<> dis(0, 999);
	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			if (dis(mRandomGenerator) <= promil) {
				Entity *entity = new Entity();
				entity->setByteCode(byteCode);
				addEntity(entity, Position(x, y));
			}
		}
	}
}

void Map::createNewEntity() {
	std::uniform_int_distribution<> baseDist(0, mEntities.size() / 2);
	Entity *baseEntity = mEntities.at(baseDist(mRandomGenerator));
	QVector<Instruction> byteCode = baseEntity->byteCode();
	byteCode.insert(qrand() % (byteCode.size()  + 1), Instruction((OpCode)(qrand() % (int)OpCode::MaxOpCode), qrand()));

	byteCode[qrand() % (byteCode.size())].mParam = qrand();
	Entity *newEntity = new Entity();
	newEntity->setByteCode(byteCode);

	Position p = findValidLocation(baseEntity->position(), 10);
	if (!p.isErrorValue()) {
		addEntity(newEntity, p);
	}
	else {
		delete newEntity;
	}

}



Tile::Tile() :
	mWaterLevel(100),
	mEntity(0) {

}

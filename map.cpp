#include "map.h"
#include "entity.h"
#include <QPainter>
#include <QFile>
#include <QDataStream>

Map::Map() :
	mTick (0),
	mWidth(0),
	mHeight(0),
	mRandomGenerator(std::random_device()()) {

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


	//If energy <= 30 then reproduce
	byteCode.append(Instruction(OpCode::LiteralPrimary, 30));
	byteCode.append(Instruction(OpCode::GetEnergy, 0));
	byteCode.append(Instruction(OpCode::CopyResultToSecondary, 0));
	byteCode.append(Instruction(OpCode::Greater, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 1));
	byteCode.append(Instruction(OpCode::Reproduce, 0));

	mDefaultByteCode = byteCode;

}

Map::Map(int width, int height) :
	mTick(0),
	mWidth(width),
	mHeight(height),
	mTiles(width * height),
	mRandomGenerator(std::random_device()()){

	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Tile &t = tile(Position(x, y));
			t.mWaterLevel = 1000;
			t.mFoodLevels[(int)FoodType::V] = 400;
		}
	}

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


	//If energy <= 30 then reproduce
	byteCode.append(Instruction(OpCode::LiteralPrimary, 40));
	byteCode.append(Instruction(OpCode::GetEnergy, 0));
	byteCode.append(Instruction(OpCode::CopyResultToSecondary, 0));
	byteCode.append(Instruction(OpCode::Greater, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 1));
	byteCode.append(Instruction(OpCode::Reproduce, 0));

	mDefaultByteCode = byteCode;
}

Map::~Map() {
	qDeleteAll(mEntities);
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

bool Map::addEntity(Entity *entity, Position pos) {
	if (isMovableLocation(pos)) {
		tile(pos).mEntity = entity;
		entity->setPosition(pos);
		mEntities.append(entity);
		return true;
	}
	return false;
}


int normalizeValue(EntityProperty property) {
	return property.sqrt().value();
}

void Map::draw(QPainter *p, int px, int py, int w, int h, int flags) {
	if (py < 0) py = 0;
	if (px < 0) px = 0;
	if (w > mWidth) w = mWidth;
	if (h > mHeight) h = mHeight;
	for (int y = py; y < h; y++) {
		for (int x = px; x < w; x++) {

			const Tile &t = tile(Position(x, y));
			int entityColor = 0;
			if (t.mEntity && (flags & NoEntities) != NoEntities) {
				entityColor = std::min(127 + normalizeValue(t.mEntity->energy()), 255);
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
			if (!t.mEntity) t.mFoodLevels[(int)FoodType::V] += 14;
			t.mFoodLevels[(int)FoodType::V] -= (t.mFoodLevels[(int)FoodType::V] / 2 + t.mFoodLevels[(int)FoodType::V]).sqrt() / 60;
			t.mFoodLevels[(int)FoodType::M] -= 10;
		}
	}
	mTick++;
}

const QList<Entity*> &Map::entities() const {
	return mEntities;
}

void Map::deletePass() {
	for (QList<Entity*>::Iterator i = mEntities.begin(); i != mEntities.end();) {
		if ((*i)->deletePass()) {
			tile((*i)->position()).mFoodLevels[(int)FoodType::M] += (*i)->energy() * 30 + 100 * sqrt(sqrt((*i)->lifeTime()));
			tile((*i)->position()).mEntity = 0;
			delete *i;
			i = mEntities.erase(i);
		}
		else {
			++i;
		}
	}
}


void Map::randomFillMapWithEntities(int promil) {
	qsrand(12314);

	std::uniform_int_distribution<> dis(0, 999);
	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			if (dis(mRandomGenerator) <= promil) {
				Entity *entity = new Entity();
				entity->setByteCode(mDefaultByteCode);
				addEntity(entity, Position(x, y));
			}
		}
	}
}

Entity *Map::createNewEntity() {
	if (mEntities.empty()) {
		Entity *entity = createDefaultEntity();
		entity->setPosition(Position(qrand() % mWidth, qrand() % mHeight));
		addEntity(entity, entity->position());
		return entity;
	}
	std::uniform_int_distribution<> baseDist(0, mEntities.size() - 1);
	Entity *baseEntity = mEntities.at(baseDist(mRandomGenerator));
	return createNewEntity(baseEntity);
}

Entity *Map::createNewEntity(Entity *baseEntity) {
	QVector<Instruction> byteCode = baseEntity->byteCode();
	if (qrand() % 10 > 4) byteCode.insert(qrand() % (byteCode.size()  + 1), Instruction((OpCode)(qrand() % (int)OpCode::MaxOpCode), qrand()));

	if (qrand() % 10 > 7) byteCode[qrand() % (byteCode.size())].mParam = qrand();

	if (qrand() % 100 > 70) {
		byteCode.removeAt(qrand() % byteCode.size());
	}

	Entity *newEntity = new Entity();
	newEntity->setGeneration(baseEntity->generation() + 1);
	newEntity->setByteCode(byteCode);

	Position p = findValidLocation(baseEntity->position() + Position(qrand() % 20 - 10, qrand() % 20 - 10), 10);
	if (!p.isErrorValue()) {
		addEntity(newEntity, p);
		return newEntity;
	}
	else {
		delete newEntity;
		return 0;
	}
}

Entity *Map::createDefaultEntity() {
	Entity *entity = new Entity();
	entity->setByteCode(mDefaultByteCode);
	return entity;
}

static const int VERSION_NUMBER = 1;
void Map::save(const QString &path) {
	QFile file(path);
	if (!file.open(QFile::WriteOnly)) {
		qDebug("Saving failed");
		return;
	}
	QDataStream out(&file);
	out << VERSION_NUMBER;
	out << mWidth;
	out << mHeight;
	out << mTick;
	out << mTiles;

	out << mEntities.size();
	for (Entity *entity : mEntities) {
		entity->save(out, VERSION_NUMBER);
	}
}

void Map::load(const QString &path) {
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		qDebug("Loading failed");
		return;
	}
	QDataStream in(&file);
	int versionNumber;
	in >> versionNumber;
	in >> mWidth;
	in >> mHeight;
	in >> mTick;
	in >> mTiles;
	int entitiesSize;
	in >> entitiesSize;
	for (int i = 0; i < entitiesSize; i++) {
		Entity *newEntity = new Entity();
		newEntity->load(in, VERSION_NUMBER);
		mEntities.append(newEntity);
		tile(newEntity->position()).mEntity = newEntity;
	}
}
quint64 Map::tick() const {
	return mTick;
}




Tile::Tile() :
	mWaterLevel(100),
	mEntity(0) {

}


QDataStream &operator <<(QDataStream &out, const Tile &tile) {
	for (int i = 0; i < (int)FoodType::MaxFoodType; i++) {
		out << tile.mFoodLevels[i];
	}
	out << tile.mWaterLevel;
	return out;
}


QDataStream &operator >>(QDataStream &in, Tile &tile) {
	for (int i = 0; i < (int)FoodType::MaxFoodType; i++) {
		in >> tile.mFoodLevels[i];
	}
	in >> tile.mWaterLevel;
	return in;
}

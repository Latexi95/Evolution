#include "map.h"
#include "entity.h"
#include <QPainter>
#include <QFile>
#include <QDataStream>
#include <cassert>

Map::Map() :
	mTick (0),
	mWidth(0),
	mHeight(0),
	mRandomGenerator(std::random_device()()) {

	mCurrentBuffer = 0;
	mDrawModes[0] = 1;
	mDrawModes[1] = 2;
	mDrawModes[2] = 3;

	initializeDefaultByteCode();

}

Map::Map(const QImage &img) :
	mTick(0),
	mWidth(img.width()),
	mHeight(img.height()),
	mTiles(img.height() * img.width()),
	mRandomGenerator(std::random_device()()) {
	assert(mWidth > 0);

	mCurrentBuffer = 0;
	mDrawModes[0] = 1;
	mDrawModes[1] = 2;
	mDrawModes[2] = 3;

	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Tile &t = tile(Position(x, y));
			t.mWaterLevel = 10;
			t.mFoodLevels[(int)FoodType::V] = 400;
			QRgb rgb = img.pixel(x, y);
			int r = (rgb & 0xFF0000) >> 16;
			int g = (rgb & 0xFF00) >> 8;
			int b = rgb & 0xFF;
			t.mWaterGenLevel = b;
			t.mFoodGenLevel = g;
			t.mHeat = r;
		}
	}

	initializeDefaultByteCode();

	mDrawBuffers[0] = QImage(mWidth, mHeight, QImage::Format_RGB32);
	mDrawBuffers[1] = QImage(mWidth, mHeight, QImage::Format_RGB32);
}

Map::~Map() {
	qDeleteAll(mEntities);
}

int Map::width() const {
	return mWidth;
}

int Map::height() const {
	return mHeight;
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

QImage Map::draw() {
	if (noDraw()) return QImage();
	QImage &curImage = mDrawBuffers[mCurrentBuffer];
	for (int y = 0; y < mHeight; y++) {
		QRgb *line = (QRgb*)curImage.scanLine(y);
		for (int x = 0; x < mWidth; x++) {

			const Tile &t = tile(Position(x, y));
			quint8 colors[3] = {0, 0, 0};
			for (int i = 0; i < 3; i++) {
				switch (mDrawModes[i]) {
					case 0:
						break;
					case 1:
						if (t.mEntity)
							colors[i] = std::min(70 + normalizeValue(t.mEntity->energy()), 255);
						break;
					case 2:
						colors[i] = normalizeValue(t.mFoodLevels[(int)FoodType::V]);
						break;
					case 3:
						colors[i] = normalizeValue(t.mFoodLevels[(int)FoodType::M]);
						break;
					case 4:
						colors[i] = normalizeValue(t.mWaterLevel);
						break;
					case 5:
						colors[i] = t.mFoodGenLevel;
						break;
					case 6:
						colors[i] = t.mWaterGenLevel;
						break;
					case 7:
						colors[i] = t.mHeat;
						break;
					case 8:
						if (t.mEntity && t.mEntity->isInBornState())
							colors[i] = 255;
					default:
						break;
				}
			}
			line[x] = qRgb(colors[0],colors[1], colors[2]);
		}
	}
	mCurrentBuffer = 1 - mCurrentBuffer;
	return curImage;
}

void Map::updateFoodLevels() {
	for (int y = 0; y < mHeight; y++) {
		for (int x = 0; x < mWidth; x++) {
			Tile &t = tile(Position(x, y));
			t.mWaterLevel += EntityProperty(t.mWaterGenLevel) - t.mWaterLevel / t.mWaterGenLevel;
			t.mFoodLevels[(int)FoodType::V] += std::max((int)sqrt(t.mFoodGenLevel * 10) - (int)t.mStressLevel.value() * t.mStressLevel.value(), 0);
			if (t.mEntity)
				t.mStressLevel += 3;
			else
				t.mStressLevel -= 1;
			t.mFoodLevels[(int)FoodType::V] -= (t.mFoodLevels[(int)FoodType::V] / 10 * t.mFoodLevels[(int)FoodType::V] / 10) / 10000;
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
			tile((*i)->position()).mFoodLevels[(int)FoodType::M] += (*i)->energy() * 40 + 30 * sqrt((*i)->lifeTime());
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

Entity *Map::createAndRandomPlaceEntity() {
	Entity *entity = 0;
	if (mEntities.empty()) {
		entity = createDefaultEntity();
	}
	else {
		std::uniform_int_distribution<> baseDist(0, mEntities.size() - 1);
		Entity *baseEntity = mEntities.at(baseDist(mRandomGenerator));
		entity = createNewEntity(baseEntity);
	}
	Position pos = findValidLocation(Position(qrand() % mWidth, qrand() % mHeight), 10);
	if (pos.isErrorValue()) {
		delete entity;
		return 0;
	}

	addEntity(entity, pos);
	return entity;
}

Entity *Map::createNewEntity(Entity *baseEntity) {
	QVector<Instruction> byteCode = baseEntity->byteCode();
	while (qrand() % 10 < 5) {
		int mod = qrand() % 10;
		if (mod < 5) {
			OpCode opCode = (OpCode)(qrand() % (int)OpCode::MaxOpCode);
			if (opCode == OpCode::Jump || opCode == OpCode::ConditionalJump) {
				byteCode.insert(qrand() % (byteCode.size()  + 1), Instruction(opCode, qrand() % 10));
			}
			else {
				byteCode.insert(qrand() % (byteCode.size()  + 1), Instruction(opCode, qrand()));
			}

		}
		else if (mod < 9 && byteCode.size() > 10) {
			byteCode.removeAt(qrand() % byteCode.size());
		}
		else {
			byteCode[qrand() % (byteCode.size())].mParam = qrand();
		}
	}
	Entity *newEntity = new Entity();
	newEntity->maxEnergy() = baseEntity->maxEnergy();
	newEntity->maxHealth() = baseEntity->maxHealth();
	newEntity->setHydrationAdaption(baseEntity->hydrationAdaption());
	newEntity->setFoodLevelAdaption(baseEntity->foodLevelAdaption());
	switch (qrand() % 30) {
		case 1:
			newEntity->maxHealth() += 1;
			break;
		case 2:
			newEntity->maxHealth() -= 1;
			break;
		case 4:
			newEntity->maxEnergy() += 1;
			break;
		case 6:
			newEntity->maxEnergy() -= 1;
			break;
		case 7:
			newEntity->setHydrationAdaption(newEntity->hydrationAdaption() + 1);
			break;
		case 8:
			newEntity->setHydrationAdaption(newEntity->hydrationAdaption() - 1);
			break;
		case 9:
			newEntity->setFoodLevelAdaption(newEntity->foodLevelAdaption() + 1);
			break;
		case 10:
			newEntity->setFoodLevelAdaption(newEntity->foodLevelAdaption() - 1);
			break;
		default:
			break;
	}

	newEntity->setGeneration(baseEntity->generation() + 1);
	newEntity->setByteCode(byteCode);
	return newEntity;
}

Entity *Map::createAndPlaceEntity(Entity *baseEntity) {
	Entity *e = createNewEntity(baseEntity);
	if (!e) return 0;
	Position p = findValidLocation(baseEntity->position(), 3);
	if (!p.isErrorValue()) {
		addEntity(e, p);
		return e;
	}
	else {
		delete e;
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

	mDrawBuffers[0] = QImage(mWidth, mHeight, QImage::Format_RGB32);
	mDrawBuffers[1] = QImage(mWidth, mHeight, QImage::Format_RGB32);
}


quint64 Map::tick() const {
	return mTick;
}

void Map::setDrawModeR(int mode) {
	mDrawModes[0] = mode;
}

void Map::setDrawModeG(int mode) {
	mDrawModes[1] = mode;
}

void Map::setDrawModeB(int mode) {
	mDrawModes[2] = mode;
}

bool Map::noDraw() const {
	return mDrawModes[0] == 0 && mDrawModes[1] == 0 && mDrawModes[2] == 0;
}

void Map::initializeDefaultByteCode() {
	QVector<Instruction> byteCode;
	byteCode.append(Instruction(OpCode::Drink, 0));
	byteCode.append(Instruction(OpCode::Eat, 0));


	byteCode.append(Instruction(OpCode::ResetTargetMarker, 0));
	byteCode.append(Instruction(OpCode::MoveTargetMarker, 0));
	byteCode.append(Instruction(OpCode::ContainsEntity, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 2));
	byteCode.append(Instruction(OpCode::Move, 0));
	byteCode.append(Instruction(OpCode::Jump, 20));

	byteCode.append(Instruction(OpCode::ResetTargetMarker, 0));
	byteCode.append(Instruction(OpCode::MoveTargetMarker, 1));
	byteCode.append(Instruction(OpCode::ContainsEntity, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 2));
	byteCode.append(Instruction(OpCode::Move, 1));
	byteCode.append(Instruction(OpCode::Jump, 13));

	byteCode.append(Instruction(OpCode::ResetTargetMarker, 0));
	byteCode.append(Instruction(OpCode::MoveTargetMarker, 2));
	byteCode.append(Instruction(OpCode::ContainsEntity, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 2));
	byteCode.append(Instruction(OpCode::Move, 2));
	byteCode.append(Instruction(OpCode::Jump, 6));

	byteCode.append(Instruction(OpCode::ResetTargetMarker, 0));
	byteCode.append(Instruction(OpCode::MoveTargetMarker, 3));
	byteCode.append(Instruction(OpCode::ContainsEntity, 0));
	byteCode.append(Instruction(OpCode::CopyResultToPrimary, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 1));
	byteCode.append(Instruction(OpCode::Move, 3));



	//If energy >= 40 then reproduce
	byteCode.append(Instruction(OpCode::LiteralPrimary, 40));
	byteCode.append(Instruction(OpCode::GetEnergy, 0));
	byteCode.append(Instruction(OpCode::CopyResultToSecondary, 0));
	byteCode.append(Instruction(OpCode::Greater, 0));
	byteCode.append(Instruction(OpCode::ConditionalJump, 1));
	byteCode.append(Instruction(OpCode::Reproduce, 0));

	mDefaultByteCode = byteCode;
}







Tile::Tile() :
	mWaterLevel(100),
	mEntity(0) {

}


QDataStream &operator <<(QDataStream &out, const Tile &tile) {
	for (int i = 0; i < (int)FoodType::MaxFoodType; i++) {
		out << tile.mFoodLevels[i];
	}
	out << tile.mStressLevel;
	out << tile.mWaterLevel;
	out << tile.mFoodGenLevel;
	out << tile.mWaterGenLevel;
	out << tile.mHeat;
	return out;
}


QDataStream &operator >>(QDataStream &in, Tile &tile) {
	for (int i = 0; i < (int)FoodType::MaxFoodType; i++) {
		in >> tile.mFoodLevels[i];
	}
	in >> tile.mStressLevel;
	in >> tile.mWaterLevel;
	in >> tile.mFoodGenLevel;
	in >> tile.mWaterGenLevel;
	in >> tile.mHeat;
	return in;
}

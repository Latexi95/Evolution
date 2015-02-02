#ifndef MAP_H
#define MAP_H
#include "entityproperty.h"
#include <QVector>
#include "position.h"
#include "enums.h"
#include <random>
#include "entity.h"

class QPainter;
struct Tile {
	Tile();

	EntityProperty foodLevel(FoodType type) const { return mFoodLevels[(int)type]; }
	EntityProperty mFoodLevels[(int)FoodType::MaxFoodType];
	EntityProperty mWaterLevel;
	Entity *mEntity;
};

enum DrawFlags {
	None = 0,
	NoEntities = 1,
	NoVFoodLevels = 2,
	NoMFoodLevels = 4,
	NoDraw = NoEntities | NoVFoodLevels | NoMFoodLevels
};

class Map {
	public:
		Map();
		Map(int width, int height);
		~Map();
		Tile &tile(Position position);
		const Tile &tile(Position position) const;
		bool isMovableLocation(Position target) const;
		bool move(Entity *entity, Position target);
		Position findValidLocation(Position nearPos, int maxRange);
		bool isPositionOnMap(Position position) const;

		bool addEntity(Entity *entity, Position pos);

		void draw(QPainter *p, int px, int py, int w, int h, int flags);

		void updateFoodLevels();
		const QList<Entity *> &entities() const;
		void deletePass();
		void randomFillMapWithEntities(int promil);

		Entity *createNewEntity();
		Entity *createNewEntity(Entity *baseEntity);

		Entity *createDefaultEntity();

		Entity *entity(Position pos) const;

		void save(const QString &path);
		void load(const QString &path);
		quint64 tick() const;

	private:
		quint64 mTick;
		int mWidth;
		int mHeight;
		QVector<Tile> mTiles;
		QList<Entity*> mEntities;
		std::mt19937 mRandomGenerator;


		QVector<Instruction> mDefaultByteCode;



};

inline bool Map::isPositionOnMap(Position position) const {
	return position.x >= 0 && position.x < mWidth && position.y >= 0 && position.y < mHeight;
}

inline Entity *Map::entity(Position pos) const {
	if (!isPositionOnMap(pos)) return 0;
	return tile(pos).mEntity;
}

QDataStream &operator << (QDataStream &out, const Tile &tile);
QDataStream &operator >> (QDataStream &in, Tile &tile);
#endif // MAP_H

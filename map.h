#ifndef MAP_H
#define MAP_H
#include "entityproperty.h"
#include <QVector>
#include "position.h"
#include "enums.h"
#include <random>

class Entity;
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
	NoMFoodLevels = 4
};

class Map {
	public:
		Map(int width, int height);
		~Map();
		Tile &tile(Position position);
		const Tile &tile(Position position) const;
		bool isMovableLocation(Position target) const;
		bool move(Entity *entity, Position target);
		Position findValidLocation(Position nearPos, int maxRange);
		bool isPositionOnMap(Position position) const;

		bool addEntity(Entity *entity, Position pos);
		void removeEntity(Entity *entity);

		void draw(QPainter *p, int px, int py, int flags);

		void updateFoodLevels();
		const QList<Entity *> &entities() const;
		void deletePass();
		void randomFillMapWithEntities(int promil);

		void createNewEntity();
	private:
		int mWidth;
		int mHeight;
		QVector<Tile> mTiles;
		QList<Entity*> mEntities;
		std::mt19937 mRandomGenerator;



};

#endif // MAP_H

#ifndef MAP_H
#define MAP_H
#include "entityproperty.h"
#include <QVector>
#include "position.h"
#include "enums.h"
#include <random>
#include "entity.h"
#include <QImage>
#include <QObject>


class QPainter;
struct Tile {
	Tile();

	EntityProperty foodLevel(FoodType type) const { return mFoodLevels[(int)type]; }
	EntityProperty mFoodLevels[(int)FoodType::MaxFoodType];
	EntityProperty mWaterLevel;
	EntityProperty mStressLevel;
	quint8 mWaterGenLevel;
	quint8 mFoodGenLevel;
	quint8 mHeat;
	Entity *mEntity;

};

class Map {
	public:
		Map();
		Map(const QImage &img);
		~Map();
		int width() const;
		int height() const;
		Tile &tile(Position position);
		const Tile &tile(Position position) const;
		bool isMovableLocation(Position target) const;
		bool move(Entity *entity, Position target);
		Position findValidLocation(Position nearPos, int maxRange);
		bool isPositionOnMap(Position position) const;

		bool addEntity(Entity *entity, Position pos);

		QImage draw();

		void updateFoodLevels();
		const QList<Entity *> &entities() const;
		void deletePass();
		void randomFillMapWithEntities(int promil);


		Entity *createAndRandomPlaceEntity();
		Entity *createNewEntity(Entity *baseEntity);

		Entity *createAndPlaceEntity(Entity *baseEntity);
		Entity *createDefaultEntity();



		Entity *entity(Position pos) const;

		void save(const QString &path);
		void load(const QString &path);
		quint64 tick() const;

		void setDrawModeR(int mode);
		void setDrawModeG(int mode);
		void setDrawModeB(int mode);
		bool noDraw() const;
	private:
		void initializeDefaultByteCode();


		quint64 mTick;
		int mWidth;
		int mHeight;
		QVector<Tile> mTiles;
		QList<Entity*> mEntities;
		std::mt19937 mRandomGenerator;

		int mDrawModes[3];
		QImage mDrawBuffers[2];
		int mCurrentBuffer;


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

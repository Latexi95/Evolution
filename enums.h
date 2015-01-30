#ifndef ENUMS
#define ENUMS
#include <QString>

enum Direction {
	Left,
	Right,
	Up,
	Down
};

enum class FoodType {
	V,
	M,
	MaxFoodType
};

inline const char *directionToString(Direction dir) {
	switch (dir) {
		case Left:
			return "Left";
		case Right:
			return "Right";
		case Up:
			return "Up";
		default:
			return "Down";
	}
}

#endif // ENUMS


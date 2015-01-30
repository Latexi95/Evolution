#include "position.h"


Position Position::targetLocation(Direction direction, int distance) const {
	switch (direction) {
		case Left:
			return Position(this->x - distance, this->y);
		case Right:
			return Position(this->x + distance, this->y);
		case Up:
			return Position(this->x, this->y - distance);
		case Down:
			return Position(this->x, this->y + distance);
		default:
			return *this;
	}
}

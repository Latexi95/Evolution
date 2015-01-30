#ifndef POSITION_H
#define POSITION_H
#include <QtGlobal>
#include "enums.h"


struct Position {
		Position() : x(0), y(0) {}
		Position(int x_, int y_) : x(x_), y(y_) {}
		static Position errorValue() { return Position(-1, -1); }

		bool operator == (Position o) const { return this->x == o.x && this->y == o.y; }
		Position operator + (Position o) const { return Position(this->x + o.x, this->y + o.y); }

		Position targetLocation(Direction direction, int distance) const;
		bool isErrorValue() const { return *this == errorValue(); }

		int x;
		int y;
};

#endif // POSITION_H

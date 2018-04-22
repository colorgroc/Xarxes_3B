#include "GlobalValues.h"

Position PixelToCell(int16_t _x, int16_t _y)
{
	int16_t xCell = _x / SIZE_CELL;
	int16_t yCell = _y / SIZE_CELL;
	return 	Position{ xCell, yCell };
}

Position CellToPixel(int16_t _x, int16_t _y)
{
	return Position{ _x * SIZE_CELL, _y * SIZE_CELL }; //convert to pixels
}

bool Walls::CheckCollision(AccumMovements accum) { //amb pixels
	bool correctPosition = true;

	for (std::vector<Position>::iterator it = obstaclesMap.begin(); it != obstaclesMap.end(); ++it) {

		if (accum.delta.x > 0) { //moviment dreta
			if ((it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).x && (it->y == PixelToCell(accum.absolute.x, accum.absolute.y).y)) || (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x && (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y))) {
				//+ SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}

		}
		else if (accum.delta.x < 0) { //moviment esquerra
			if ((it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x && (it->y == PixelToCell(accum.absolute.x, accum.absolute.y).y)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x))) {
				// - SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}

		if (accum.delta.y > 0) { //moviment baix
			if ((it->y == PixelToCell(accum.absolute.x , accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x))) {
				// + SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}
		else if (accum.delta.y < 0) { //moviment dalt
			if ((it->y == PixelToCell(accum.absolute.x , accum.absolute.y).y && (it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).x))) {
				//- SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}
	}
	return correctPosition;
}

bool Walls::CheckCollision(Position pos) { //amb pixels
	bool correctPosition = true;

	for (std::vector<Position>::iterator it = obstaclesMap.begin(); it != obstaclesMap.end(); ++it) {

		if(pos.x == it->x && pos.y == it->y)
			correctPosition = false;
	}
	return correctPosition;
}

sf::Packet& operator <<(sf::Packet& Packet, const Position& pos)
{
return Packet << pos.x << pos.y;
}

sf::Packet& operator >>(sf::Packet& Packet, Position& pos)
{
return Packet >> pos.x >> pos.y;
}

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum) {
	return Packet << accum.delta << accum.absolute;
}

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum) {
	return Packet >> accum.delta >> accum.absolute;
}
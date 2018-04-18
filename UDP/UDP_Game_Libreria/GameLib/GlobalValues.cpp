#include "GlobalValues.h"

Position PixelToCell(int8_t _x, int8_t _y)
{
	int8_t xCell = _x / SIZE_CELL;
	int8_t yCell = _y / SIZE_CELL;
	return 	Position{ xCell, yCell };
}

Position CellToPixel(int8_t _x, int8_t _y)
{
	return Position{ _x * SIZE_CELL, _y * SIZE_CELL };; //convert to pixels
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
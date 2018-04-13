#include "GlobalValues.h"

//namespace _cmd {
//	//comandos
//	int8_t _cmd::HELLO = 0;
//	int8_t _cmd::ACK_HELLO = 1;
//	int8_t _cmd::NEW_CONNECTION = 2;
//	int8_t _cmd::ACK_NEW_CONNECTION = 3;
//	int8_t _cmd::DISCONNECTION = 4;
//	int8_t _cmd::ACK_DISCONNECTION = 5;
//	int8_t _cmd::PING = 6;
//	int8_t _cmd::ACK_PING = 7;
//
//	sf::IpAddress _cmd::serverIP = "localhost";
//	unsigned short serverPORT = PORT;
//}

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
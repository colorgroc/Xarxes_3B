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

sf::Vector2f GetCell(int8_t _x, int8_t _y)
{
	float xCell = _x / SIZE_CELL;
	float yCell = _y / SIZE_CELL;
	sf::Vector2f cell(xCell, yCell);
	return cell;
}

sf::Vector2f BoardToWindows(sf::Vector2f _positionCell)
{
	return sf::Vector2f(_positionCell.x * SIZE_CELL, _positionCell.y * SIZE_CELL); //convert to pixels
}

/*sf::Packet& operator <<(sf::Packet& Packet, const Position& pos)
{
return Packet << pos.x << pos.y;
}

sf::Packet& operator >>(sf::Packet& Packet, Position& pos)
{
return Packet >> pos.x >> pos.y;
}*/
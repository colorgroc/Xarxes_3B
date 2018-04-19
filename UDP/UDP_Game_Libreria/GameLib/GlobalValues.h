#pragma once
#include <stdint.h>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 4
#define MAX_OPPONENTS 3

#define _PING 1000
#define SENDING_PING 500
#define CONTROL_PING 5000
#define SEND_ACCUMMOVEMENTS 100

#define PORT 50000

#define WINDOW_SIZE 500
#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f
#define TOP_LIMIT 0
#define	LOW_LIMIT 500
#define RIGHT_LIMIT 500
#define LEFT_LIMIT 0
#define PIXELSTOMOVE 3

enum Cmds {
	HELLO, ACK_HELLO, NEW_CONNECTION, ACK_NEW_CONNECTION, DISCONNECTION, ACK_DISCONNECTION, PING, ACK_PING, TRY_POSITION, OK_POSITION, REFRESH_POSITIONS, ACK_REFRESH_POSITIONS
};

struct Position {
	int16_t x;
	int16_t y;
};

//llista de de walls en cel·les
struct Walls
{
	std::vector<Position> walls = { Position{ 5,5 }, Position{ 6,5 }, Position{ 7,5 } };
};


struct AccumMovements {
	Position delta;
	Position absolute;
};

struct Client {
	int32_t id;
	std::string nickname;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	bool connected;
	std::map<int32_t, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
	std::map<int32_t, AccumMovements> MapAccumMovements; 		//	idmovement per controlar validacions,
																//	moviments acumulats
};

struct Player
{
	int32_t ID = 0;
	std::string nickname;
	Position position;
	std::map<int32_t, sf::Packet> resending;
	std::map<int32_t, AccumMovements> MapAccumMovements;
};



Position PixelToCell(int16_t _x, int16_t _y);

Position CellToPixel(int16_t _x, int16_t _y);

sf::Packet& operator <<(sf::Packet& Packet, const Position& pos);

sf::Packet& operator >>(sf::Packet& Packet, Position& pos);

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum);

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum);
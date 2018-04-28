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
#include <random>
#include <math.h>
#include <queue>    
#include <set>

#define MAX_CLIENTS 4
#define MAX_OPPONENTS 3

#define _PING 1000
#define SENDING_PING 250
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
#define PIXELSTOMOVE 1

#define PERCENT_PACKETLOSS 0.05


enum Cmds {
	HELLO, ACK_HELLO, NEW_CONNECTION, ACK_NEW_CONNECTION, DISCONNECTION, ACK_DISCONNECTION, PING, ACK_PING, TRY_POSITION, NOT_OK_POSITION, OK_POSITION, REFRESH_POSITIONS, ACK_REFRESH_POSITIONS, TRY_COLLISION_OPPONENT, QUI_LA_PILLA, ACK_QUI_LA_PILLA, GAMESTARTED, WINNER, ACK_WINNER, ID_ALREADY_TAKEN
};

struct Position {
	int16_t x;
	int16_t y;
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
	bool laPara;
	bool winner;
	std::map<int32_t, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
	std::map<int32_t, AccumMovements> MapAccumMovements; 		//	idmovement per controlar validacions,
																//	moviments acumulats
};

struct Player
{
	int32_t ID = 0;
	std::string nickname;
	bool laParo;
	Position position;
	std::map<int32_t, sf::Packet> resending;
	std::map<int32_t, AccumMovements> MapAccumMovements;
};
struct InterpolationAndStuff {
	Position lastPos;
	Position newPos;
	bool laPara;
	std::queue<Position> middlePositions; //primer a entrar primer a sortir (fifo)
};

Position PixelToCell(int16_t _x, int16_t _y);

Position CellToPixel(int16_t _x, int16_t _y);


//llista de de walls en cel�les
class Walls
{
public:
	std::vector<Position> obstaclesMap;

	Walls() {
		//obstacles
		obstaclesMap = { Position{ 5,5 }, Position{ 6,5 }, Position{ 7,5 },  Position{ 7,6 }, Position{ 7,7 }, Position{ 7,8 },  Position{ 7,9 }, Position{ 7,10 }, Position{ 7,11 },Position{ 7,12 }, Position{ 7,13 }, Position{ 7,14 },
			Position{ 8,19 }, Position{ 9,19 }, Position{ 10,19},  Position{ 11,19 }, Position{ 12,19 }, Position{ 13,19 },
			Position{ 16,8 }, Position{ 16,9 }, Position{ 16,10 },  Position{ 16,11 }, Position{ 16,12 }, Position{ 16,13 }, };

		for (int8_t i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (int8_t j = 0; j < NUMBER_ROWS_COLUMNS; j++)
			{
				if (i == 0 || i == NUMBER_ROWS_COLUMNS - 1 || j == 0 || j == NUMBER_ROWS_COLUMNS-1) {
					obstaclesMap.push_back(Position{ i,j });
				}
			}
		}
	}

	bool CheckCollision(AccumMovements accum);
	bool Walls::CheckCollision(Position pos);

	~Walls() {
		obstaclesMap.clear();
	}
};

static float GetRandomFloat() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	//static std::uniform_real_distribution<float>dis(since, to);
	static std::uniform_real_distribution<float>dis(0.f, 1.f);
	return dis(gen);
}



sf::Packet& operator <<(sf::Packet& Packet, const Position& pos);

sf::Packet& operator >>(sf::Packet& Packet, Position& pos);

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum);

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum);
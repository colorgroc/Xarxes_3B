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

////comandos
//namespace _cmd {
//	int8_t HELLO;
//	int8_t ACK_HELLO;
//	int8_t NEW_CONNECTION;
//	int8_t ACK_NEW_CONNECTION;
//	int8_t DISCONNECTION;
//	int8_t ACK_DISCONNECTION;
//	int8_t PING;
//	int8_t ACK_PING;
//  int8_t TRY_POSITION = 8;
//  int8_t OK_POSITION = 9;

//	sf::IpAddress serverIP;
//	unsigned short serverPORT;
//}

struct Position {
	int16_t x;
	int16_t y;
};

struct AccumMovements {
	Position delta;
	Position absolute;
};

struct Client {
	int8_t id;
	std::string nickname;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	bool connected;
	std::map<int8_t, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
	std::map<int8_t, std::pair<int8_t, AccumMovements>> MapAccumMovements; //idpacket rebut de part del client (utilitzat per despres borrarlo del resend del client), 
																//	idmovement per controlar validacions,
																//	moviments acumulats
};

struct Player
{
	int8_t ID = 0;
	std::string nickname;
	Position position;
	std::map<int8_t, sf::Packet> resending;
	std::map<int8_t, AccumMovements> MapAccumMovements;
};


Position PixelToCell(int8_t _x, int8_t _y);

Position CellToPixel(int8_t _x, int8_t _y);

sf::Packet& operator <<(sf::Packet& Packet, const Position& pos);

sf::Packet& operator >>(sf::Packet& Packet, Position& pos);

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum);

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum);
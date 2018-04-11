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

#define MAX_CLIENTS 2
#define MAX_OPPONENTS 3

#define _PING 1000
#define SENDING_PING 500
#define CONTROL_PING 2000

#define PORT 50000

#define WINDOW_SIZE 500
#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f

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
//
//	sf::IpAddress serverIP;
//	unsigned short serverPORT;
//}

struct Position {
	int8_t x;
	int8_t y;
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
};

struct Player
{
	int8_t ID = 0;
	std::string nickname;
	Position position;
	std::map<int8_t, sf::Packet> resending;
};


sf::Vector2f GetCell(int8_t _x, int8_t _y);

sf::Vector2f BoardToWindows(sf::Vector2f _positionCell);

sf::Packet& operator <<(sf::Packet& Packet, const Position& pos);

sf::Packet& operator >>(sf::Packet& Packet, Position& pos);
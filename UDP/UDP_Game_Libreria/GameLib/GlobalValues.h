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

#define _PING 5000
#define SENDING_PING 1000
#define CONTROL_PING 10000

#define PORT 50000

#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f

//comandos
int8_t HELLO = 0;
int8_t ACK_HELLO = 1;
int8_t NEW_CONNECTION = 2;
int8_t ACK_NEW_CONNECTION = 3;
int8_t DISCONNECTION = 4;
int8_t ACK_DISCONNECTION = 5;
int8_t PING = 6;
int8_t ACK_PING = 7;

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;

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
	Position position;
	std::map<int8_t, sf::Packet> resending;
};


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
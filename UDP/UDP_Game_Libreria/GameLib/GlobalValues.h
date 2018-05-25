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
#define _PING_LOBBY 5000
#define SENDING_PING 250
#define CONTROL_PING 5000
#define CONTROL_PING_LOBBY 20000
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

#define PERCENT_PACKETLOSS 0

enum Cmds {
	WELCOME, NEW_CONNECTION, ACK_NEW_CONNECTION, DISCONNECTION, ACK_DISCONNECTION, PING, PING_LOBBY, ACK_PING, ACK_PING_LOBBY, TRY_POSITION, OK_POSITION, REFRESH_POSITIONS, TRY_COLLISION_OPPONENT, QUI_LA_PILLA, ACK_QUI_LA_PILLA, GAMESTARTED, WINNER, ACK_WINNER, ID_ALREADY_TAKEN, NEW_GAME, JOIN_GAME, PASSWORD_INCORRECT, LOGIN, SIGNUP, ACK_LOGIN, ACK_SIGNUP, ID_ALREADY_CONNECTED, GLOBAL_CHAT, GAME_CHAT, NEW_CONNECTION_LOBBY, DISCONNECTION_LOBBY, ACK_DISCONNECTION_LOBBY, ACK_NEW_CONNECTION_LOBBY, ID_ALREADY_PLAYING, NEW_GAME_CREATED, GAME_DELETED
};
struct Chat {
	int8_t id;
	std::string nickname;
	std::string mensaje;
};
struct Position {
	int16_t x;
	int16_t y;
};

struct AccumMovements {
	Position delta;
	Position absolute;
};

struct ClientLobby {
	int32_t id;
	std::string nickname;
	sf::IpAddress ip;
	unsigned short port;
	int8_t connexions;
	int8_t desconnexions;
	int8_t partidesjugades;
	bool connected;
	std::map<int32_t, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
};
struct Player {
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

//struct Partida {
//	int8_t id;
//	GameClient owner;
//	std::string name;
//	std::string password;
//	int8_t maxPlayers;
//	//int8_t numPlayersConnected;
//	std::map<int32_t, GameClient> jugadors;
//};

struct PartidaClient {
	int32_t id;
	std::string name;
	int32_t numPlayersConnected;
	int32_t maxPlayers;
};

struct Jugador
{
	int32_t ID = 0;
	int32_t IDPartida;
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
	std::string nickname;
	std::queue<Position> middlePositions; //primer a entrar primer a sortir (fifo)
};

Position PixelToCell(int16_t _x, int16_t _y);

Position CellToPixel(int16_t _x, int16_t _y);

class Partida {
public:
	int32_t id;
	int32_t idOwner;
	std::string name;
	std::string password;
	int32_t maxPlayers;
	bool gameStarted = false;
	bool once = false;
	int32_t packetID = 1;
	int32_t receivedWinner = 0;
	sf::Clock clockPositions;
	sf::Clock clockPing, clockSend;
	std::set<int32_t> pillados;
	std::map<int32_t, Player> jugadors;

	Partida() {};
	~Partida() { jugadors.clear(); pillados.clear(); };
	Partida(int32_t id, int32_t idOwner, std::string name, std::string password, int32_t maxPlayers);
	bool CheckCollisionWithClientsPos(Position pos);
	void ComprovacioPillats();

};

//llista de de walls en cel·les
class Walls
{
public:
	std::vector<Position> obstaclesMap;

	Walls() {
		//obstacles
		obstaclesMap = { Position{ 5,5 }, Position{ 6,5 }, Position{ 7,5 },  Position{ 7,6 }, Position{ 7,7 }, Position{ 7,8 },  Position{ 7,9 }, Position{ 7,10 }, Position{ 7,11 },Position{ 7,12 }, Position{ 7,13 }, Position{ 7,14 },
			Position{ 8,19 }, Position{ 9,19 }, Position{ 10,19 },  Position{ 11,19 }, Position{ 12,19 }, Position{ 13,19 },
			Position{ 16,8 }, Position{ 16,9 }, Position{ 16,10 },  Position{ 16,11 }, Position{ 16,12 }, Position{ 16,13 }, };

		for (int8_t i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (int8_t j = 0; j < NUMBER_ROWS_COLUMNS; j++)
			{
				if (i == 0 || i == NUMBER_ROWS_COLUMNS - 1 || j == 0 || j == NUMBER_ROWS_COLUMNS - 1) {
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
	static std::uniform_real_distribution<float>dis(0.f, 1.f);
	return dis(gen);
}



bool SortByNameDown(const PartidaClient &a, const PartidaClient &b);
bool SortByConnectionDown(const PartidaClient &a, const PartidaClient &b);
bool SortByMaxNumDown(const PartidaClient &a, const PartidaClient &b);
bool SortByNameUp(const PartidaClient &a, const PartidaClient &b);
bool SortByConnectionUp(const PartidaClient &a, const PartidaClient &b);
bool SortByMaxNumUp(const PartidaClient &a, const PartidaClient &b);


sf::Packet& operator <<(sf::Packet& Packet, const Position& pos);

sf::Packet& operator >>(sf::Packet& Packet, Position& pos);

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum);

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum);
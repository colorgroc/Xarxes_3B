//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

#define MAX_OPPONENTS 3
#define CONNECTION 1
#define DISCONNECTION 2
#define MOVE_UP 3
#define MOVE_DOWN 4
#define MOVE_RIGHT 5
#define MOVE_LEFT 6
#define PORT 50000

#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
int state = 1;
unsigned short port;
sf::UdpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;

struct Position {
	int x;
	int y;
};

struct Player
{
	int id;
	Position position;
};

Player * myPlayer;
std::map <int, Position*> opponents;
//Position opponents[MAX_OPPONENTS - 1];


sf::Vector2f GetCell(int _x, int _y)
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

void Send(int action) {
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	sf::Packet packet;
	if (action == DISCONNECTION) {
		packet << DISCONNECTION << myPlayer->id;
	}
	else if (action == MOVE_UP) {

	}
	else if (action == MOVE_DOWN) {

	}
	else if (action == MOVE_RIGHT) {

	}
	else if (action == MOVE_LEFT) {

	}
	status = socket.send(packet, "localhost", PORT);
	if (status == sf::Socket::Error) std::cout << "Error" << std::endl;
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Server disconnected" << std::endl;
		socket.unbind();
	}

}

void ReceiveData() {
	//fer mutex/thread o nonblocking
	sf::Packet packet;
	int action;
	int opponentId;
	status = socket.receive(packet, serverIP, serverPORT);
	if (status == sf::Socket::Done) {
		packet >> action >> opponentId;

		if (action == CONNECTION) {
			Position pos;
			packet >> pos.x >> pos.y;
			std::cout << "A new opponent connected. ID: " << opponentId << " Position: " << pos.x << ", " << pos.y << std::endl;
			//opponents.insert(std::make_pair(opponentId, new Position{ pos.x, pos.y }));
			opponents.insert(std::make_pair(opponentId, &pos));
		}
		if (action == DISCONNECTION) {
			std::cout << "An opponent disconnected. ID: " << opponentId << std::endl;
			opponents.erase(opponentId);
		}

	}
}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(500, 500), "Traffic Game");
	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();
		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				Send(DISCONNECTION);
				socket.unbind();
				window.close();
				break;

			default:
				break;

			}
		}

		//clearing window and drawing again
		window.clear();

		for (int i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (int j = 0; j < NUMBER_ROWS_COLUMNS; j++)
			{
				sf::RectangleShape rectBlanco(sf::Vector2f(SIZE_CELL, SIZE_CELL));
				rectBlanco.setFillColor(sf::Color::White);
				if (i % 2 == 0)
				{
					if (j % 2 == 0)
					{
						rectBlanco.setPosition(sf::Vector2f(i*SIZE_CELL, j*SIZE_CELL));
						window.draw(rectBlanco);
					}
				}
				else
				{
					if (j % 2 == 1)
					{
						rectBlanco.setPosition(sf::Vector2f(i*SIZE_CELL, j*SIZE_CELL));
						window.draw(rectBlanco);
					}
				}
			}
		}

		//draw the player circle
		sf::CircleShape shapePlayer(RADIUS_SPRITE);
		shapePlayer.setFillColor(sf::Color::Green);

		sf::Vector2f positionPlayer(myPlayer->position.x, myPlayer->position.y);
		positionPlayer = BoardToWindows(positionPlayer);
		shapePlayer.setPosition(positionPlayer);

		window.draw(shapePlayer);

		//draw the opponents circle
		sf::CircleShape shapeOpponent(RADIUS_SPRITE);
		shapeOpponent.setFillColor(sf::Color::Red);

		for (int i = 0; i < MAX_OPPONENTS; i++) {
			for (std::map<int, Position*>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
				//sf::Vector2f positionOpponent(opponents[i].x, opponents[i].y);
				sf::Vector2f positionOpponent(it->second->x, it->second->y);
				positionOpponent = BoardToWindows(positionOpponent);
				shapeOpponent.setPosition(positionOpponent);

				window.draw(shapeOpponent);
			}
		}

		window.display();
	}

}


void InitialReceive() {
	sf::Packet packet;
	std::string msg;
	status = socket.receive(packet, serverIP, serverPORT);
	if (status == sf::Socket::Done) {
		packet >> msg >> myPlayer->id >> myPlayer->position.x >> myPlayer->position.y;
		std::cout << msg << "Client ID: " << myPlayer->id << " Initial Position: " << myPlayer->position.x << ", " << myPlayer->position.y << std::endl;
	}
}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	port = socket.getLocalPort();
	myPlayer = new Player();
	socket.setBlocking(false);
	//initial connection
	std::cout << "Estableciendo conexion con server... \n";
	Send(CONNECTION);
	InitialReceive();

	GameManager();

	//socket.disconnect();
	system("pause");

	return 0;
}
//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

#define MAX_OPPONENTS 3
#define PORT 50000

#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
int state = 1;
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
std::map <int, Position> opponents;

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

void Send(std::string cmd) {
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	sf::Packet packet;
	if (cmd == "DISCONNECTION") {
		packet << "DISCONNECTION" << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;
	}
	if (cmd == "ACK_PING") {
		packet << "ACK_PING" << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;

	}
	

}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	std::string cmd;
	int opponentId;
	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		packet >> cmd >> opponentId;

		if (cmd == "CONNECTION") {
			Position pos;
			packet >> pos.x >> pos.y;
			std::cout << "A new opponent connected. ID: " << opponentId << " Position: " << pos.x << ", " << pos.y << std::endl;
			opponents.insert(std::make_pair(opponentId, pos));
		}
		else if (cmd == "DISCONNECTION") {
			std::cout << "An opponent disconnected. ID: " << opponentId << std::endl;
			opponents.erase(opponentId);
		}
		else if (cmd == "POSITION") { //update all positions
			Position pos;
			packet >> pos.x >> pos.y;

			if (opponentId == myPlayer->id) {
				myPlayer->position = pos; //soc jo
			}
			else if (opponents.find(opponentId) == opponents.end()) {
				//encara no esta a la llista
				opponents.insert(std::make_pair(opponentId, pos));
			}
			else {
				//esta a la llista i no soc jo
				opponents.find(opponentId)->second = pos;
			}
		}
		else if (cmd == "PING") {
			Send("ACK_PING");
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
				Send("DISCONNECTION");
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
				sf::Color grey = sf::Color(49, 51, 53);
				rectBlanco.setFillColor(grey);
				//rectBlanco.setOutlineColor(sf::Color::Green);
				//rectBlanco.setOutlineThickness(2.f);
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

		for (std::map<int, Position>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
			sf::Vector2f positionOpponent(it->second.x, it->second.y);
			positionOpponent = BoardToWindows(positionOpponent);
			shapeOpponent.setPosition(positionOpponent);

			window.draw(shapeOpponent);
		}

		window.display();
	}

}


void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";

	sf::Packet packet;
	packet << "NEWCONNECTION";
	status = socket.send(packet, "localhost", PORT);
	if (status == sf::Socket::Error) std::cout << "Error" << std::endl;
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Server disconnected" << std::endl;
		socket.unbind();
	}

	packet.clear();
	std::string cmd;
	status = socket.receive(packet, serverIP, serverPORT);
	if (status == sf::Socket::Done) {
		packet >> cmd;
		if (cmd == "WELCOME") {
			packet >> myPlayer->id >> myPlayer->position.x >> myPlayer->position.y;
			std::cout << "WELCOME! " << "Client ID: " << myPlayer->id << " Initial Position: " << myPlayer->position.x << ", " << myPlayer->position.y << std::endl;
		}
	
	}

	socket.setBlocking(false);
}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	myPlayer = new Player();

	//initial connection
	ConnectionWithServer();

	GameManager();

	//socket.disconnect();
	system("pause");

	return 0;
}
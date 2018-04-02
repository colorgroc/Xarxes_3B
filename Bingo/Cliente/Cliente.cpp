//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#define MAX_OPPONENTS 3
#define RECEIVED 1
#define WRITED 2
#define CONNECTION 3
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
Position opponents[MAX_OPPONENTS - 1];


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


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(500, 500), "Traffic Game");
	while (window.isOpen())
	{
		sf::Event event;

		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
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
			sf::Vector2f positionOpponent(opponents[i].x, opponents[i].y);
			positionOpponent = BoardToWindows(positionOpponent);
			shapeOpponent.setPosition(positionOpponent);

			window.draw(shapeOpponent);
		}

		window.display();
	}

}


void Send() {

	sf::Packet packet;
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	status = socket.send(packet, "localhost", PORT);
	if (status == sf::Socket::Error) std::cout << "Error" << std::endl;
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Server disconnected" << std::endl;
		socket.unbind();
	}

}

void Receive() {
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
	myPlayer = new Player();

	//initial connection
	std::cout << "Estableciendo conexion con server... \n";
	Send();
	Receive();

	GameManager();

	//socket.disconnect();
	system("pause");

	return 0;
}
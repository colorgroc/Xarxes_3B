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
#define SENDING_PING 1000
#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define TOP_LIMIT 0
#define	LOW_LIMIT 25
#define RIGHT_LIMIT 25
#define LEFT_LIMIT 0
#define RADIUS_SPRITE 10.0f

//comandos
sf::Int8 HELLO = 0;
sf::Int8 ACK_HELLO = 1;
sf::Int8 NEW_CONNECTION = 2;
sf::Int8 ACK_NEW_CONNECTION = 3;
sf::Int8 DISCONNECTION = 4;
sf::Int8 ACK_DISCONNECTION = 5;
sf::Int8 PING = 6;
sf::Int8 ACK_PING = 7;
sf::Int8 TRY_POSITION = 8;
sf::Int8 OK_POSITION = 9;

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
sf::Int8 state = 1;
sf::UdpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
//bool once = false;
sf::Int8 packetID = 1;
sf::Clock c;

struct Position {
	sf::Int8 x;
	sf::Int8 y;
};

struct Player
{
	sf::Int8 ID = 0;
	Position position;
	std::map<sf::Int8, sf::Packet> resending;
};

Player * myPlayer;
std::map <sf::Int8, Position> opponents;

/*sf::Packet& operator <<(sf::Packet& Packet, const Position& pos)
{
	return Packet << pos.x << pos.y;
}

sf::Packet& operator >>(sf::Packet& Packet, Position& pos)
{
	return Packet >> pos.x >> pos.y;
}*/

sf::Vector2f PixelsToCell(sf::Int8 _x, sf::Int8 _y)
{
	float xCell = _x / SIZE_CELL;
	float yCell = _y / SIZE_CELL;
	sf::Vector2f cell(xCell, yCell);
	return cell;
}

sf::Vector2f CellToPixels(sf::Vector2f _positionCell)
{
	return sf::Vector2f(_positionCell.x * SIZE_CELL, _positionCell.y * SIZE_CELL); //convert to pixels
}

void Resend() {
	//posar mutex??

	for (std::map<sf::Int8, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
		status = socket.send(msg->second, "localhost", PORT);
		if (status == sf::Socket::Error) {
			std::string cmd;
			msg->second >> cmd;
			std::cout << "Error sending the message. Client to Server." << "Message IP: " << std::to_string(msg->first) << "Message: " << cmd << std::endl;
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Error sending the message. Server disconnected." << std::endl;
			//connected = false;
			socket.unbind();
			system("exit");
		}
	}
}

void SendACK(sf::Int8 cmd, sf::Int8 pID) {
	sf::Packet packet;
	std::string com;

	if (cmd == ACK_DISCONNECTION) {
		com = "DISCONNECTION";
	}
	else if (cmd == ACK_NEW_CONNECTION) {
		com = "NEW_CONNECTION";
	}
	else if (cmd == ACK_PING) {
		com = "ACK_PING";
	}

	packet << cmd << pID << myPlayer->ID;
	status = socket.send(packet, "localhost", PORT);
	if (status == sf::Socket::Error) std::cout << "Error. " << com << std::endl;
	packet.clear();
}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::Int8 cmd = 0;
	sf::Int8 opponentId = 0;
	sf::Int8 packetIDRecived = 0;
	
	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		packet >> cmd >> packetIDRecived;

		if (cmd == PING) {	
			SendACK(ACK_PING, packetIDRecived);
		}
		else if (cmd == ACK_HELLO) {
			//std::cout << "ACK_HELLO recived." << std::endl;
			if (myPlayer->ID == 0) {
				sf::Int8 numOfOpponents = 0;
				packet >> myPlayer->ID >> myPlayer->position.x >> myPlayer->position.y >> numOfOpponents;
				if (numOfOpponents > 0) {
					//treiem del packet la ID i la pos de cada oponent
					for (int i = 0; i < numOfOpponents; i++) {
						sf::Int8 oID;
						Position oPos;
						packet >> oID >> oPos.x >> oPos.y;
						opponents.insert(std::make_pair(oID, oPos));
					}
				}
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
				std::cout << "WELCOME! " << " Client ID: " << std::to_string(myPlayer->ID) << " Initial Position: " << std::to_string(myPlayer->position.x) << ", " << std::to_string(myPlayer->position.y) << std::endl;
			}
		}

		//std::cout << std::to_string(opponentId) << std::endl;
		else if (cmd == NEW_CONNECTION) {
			packet >> opponentId;
			if (opponents.find(opponentId) == opponents.end()) {
				Position pos;
				packet >> pos.x >> pos.y;
				std::cout << "A new opponent connected. ID: " << std::to_string(opponentId) << " Position: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << " PacketID Server: " << std::to_string(packetIDRecived) << std::endl;
				opponents.insert(std::make_pair(opponentId, pos));
			}
			SendACK(ACK_NEW_CONNECTION, packetIDRecived);

		}
		else if (cmd == DISCONNECTION) {
			packet >> opponentId;
			if (opponents.find(opponentId) != opponents.end()) {
				std::cout << "An opponent disconnected. ID: " << std::to_string(opponentId) << " PacketID Server: " << std::to_string(packetIDRecived) << std::endl;
				opponents.erase(opponentId);
			}
			SendACK(ACK_DISCONNECTION, packetIDRecived);
		}

	} packet.clear();
}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(500, 500), "Traffic Game");
	//c.restart();

	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();
		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}
		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				socket.unbind();
				window.close();
				break;
			case  sf::Event::KeyPressed: //el moviment en aquesta versio es per celes
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) //moure personatge esquerra
				{
					if (myPlayer->position.x != LEFT_LIMIT) {
						sf::Packet packet;
						packet << TRY_POSITION << packetID << myPlayer->ID << myPlayer->position.x - 1 << myPlayer->position.y; //poner packetID
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						//myPlayer->position.x = myPlayer->position.x - 1;
					}
				
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) //moure personatge dreta
				{
					if (myPlayer->position.x != RIGHT_LIMIT-1) {
						myPlayer->position.x = myPlayer->position.x + 1;
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) //moure personatge dalt
				{
					if (myPlayer->position.y != TOP_LIMIT) {
						myPlayer->position.y = myPlayer->position.y - 1;
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) //moure personatge baix
				{
					if (myPlayer->position.y != LOW_LIMIT-1) {
						myPlayer->position.y = myPlayer->position.y + 1;
					}
				}
			
			default:
				break;

			}
		}

		//clearing window and drawing again
		window.clear();

		for (sf::Int8 i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (sf::Int8 j = 0; j < NUMBER_ROWS_COLUMNS; j++)
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
		positionPlayer = CellToPixels(positionPlayer);
		shapePlayer.setPosition(positionPlayer);

		window.draw(shapePlayer);

		//draw the opponents circle
		sf::CircleShape shapeOpponent(RADIUS_SPRITE);
		shapeOpponent.setFillColor(sf::Color::Red);

		for (std::map<sf::Int8, Position>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
			sf::Vector2f positionOpponent(it->second.x, it->second.y);
			positionOpponent = CellToPixels(positionOpponent);
			shapeOpponent.setPosition(positionOpponent);

			window.draw(shapeOpponent);
		}

		window.display();
	}

}


void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";
	std::string nickname;
	std::cout << "Type your nickname: ";
	std::getline(std::cin, nickname);
	sf::Packet packet;
	packet << HELLO << packetID << nickname; //poner packetID
	myPlayer->resending.insert(std::make_pair(packetID, packet));
	packetID++;
	packet.clear();
}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	socket.setBlocking(false);

	myPlayer = new Player();

	//initial connection
	ConnectionWithServer();

	GameManager();

	//socket.disconnect();
	system("exit");

	return 0;
}



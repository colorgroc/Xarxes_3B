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
#define SENDING_PING 500
#define SIZE_CELL 20
#define NUMBER_ROWS_COLUMNS 25
#define RADIUS_SPRITE 10.0f

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
int8_t state = 1;
sf::UdpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
//bool once = false;
int8_t packetID = 1;
sf::Clock c;

struct Position {
	int8_t x;
	int8_t y;
};

struct Player
{
	int8_t ID = -1;
	Position position;
	std::map<int8_t, sf::Packet> resending;
};

Player * myPlayer;
std::map <int8_t, Position> opponents;

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

void Resend() {
	//posar mutex??

	for (std::map<int8_t, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
		status = socket.send(msg->second, serverIP, serverPORT);
		if (status == sf::Socket::Error)
			std::cout << "Error sending the message. Client to Server." << std::endl;
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Error sending the message. Server disconnected." << std::endl;
			//connected = false;
			socket.unbind();
			system("exit");
		}
	}
}

void Send(std::string cmd) {
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	sf::Packet packet;
	//int packetIDRecived;

	if (cmd == "DISCONNECTION") {
		packet << "DISCONNECTION" << myPlayer->ID;
		//myPlayer->resending.insert(std::make_pair(-1, packet));
		status = socket.send(packet, serverIP, serverPORT);
		if (status == sf::Socket::Error) std::cout << "Disconnection Error." << std::endl;
	}
	else if (cmd == "ACK_PING") {
		packet << "ACK_PING" << myPlayer->ID;
		//myPlayer->resending.insert(std::make_pair(0, packet));
		status = socket.send(packet, serverIP, serverPORT);
		if (status == sf::Socket::Error) std::cout << "ACK Ping Error." << std::endl;

	}


}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	std::string cmd;
	int8_t opponentId = 0;
	int8_t packetIDRecived = 0;
	packet.clear();
	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		packet >> cmd;
		if (cmd == "PING") {
			Send("ACK_PING");
		}
		else {
			packet >> packetIDRecived;
			if (cmd == "WELCOME") {
				if (myPlayer->ID == -1) {
					packet >> myPlayer->ID >> myPlayer->position.x >> myPlayer->position.y;

					//if (!once) {
					//once = true;

					std::cout << "WELCOME! " << "Server Packet: " << std::to_string(packetIDRecived) << " Client ID: " << std::to_string(myPlayer->ID) << " Initial Position: " << std::to_string(myPlayer->position.x) << ", " << std::to_string(myPlayer->position.y) << std::endl;
					//}
				}
			}
			else if (cmd == "ACK") {
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
			}
			else {
				packet >> opponentId;

				if (cmd == "CONNECTION") {
					if (opponents.find(opponentId) == opponents.end()) {
						Position pos;
						packet >> pos.x >> pos.y;
						std::cout << "A new opponent connected. ID: " << std::to_string(opponentId) << " Position: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << " PacketID Server: " << packetIDRecived << std::endl;
						opponents.insert(std::make_pair(opponentId, pos));
					}
				}
				else if (cmd == "DISCONNECTION") {
					if (opponents.find(opponentId) != opponents.end()) {
						std::cout << "An opponent disconnected. ID: " << std::to_string(opponentId) << " PacketID Server: " << std::to_string(packetIDRecived) << std::endl;
						opponents.erase(opponentId);
					}
				}
				else if (cmd == "POSITION") { //update all positions
					Position pos;
					packet >> pos.x >> pos.y;

					if (opponentId == myPlayer->ID) {
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
				packet.clear();
				packet << "ACK" << packetIDRecived << myPlayer->ID;
				if (myPlayer->resending.find(packetIDRecived) == myPlayer->resending.end())
					myPlayer->resending.insert(std::make_pair(packetIDRecived, packet));
				//fer resend
			}
		}
	}
	//mirar si hem rebut algun packet amb un id superior
	/* for (std::map<int, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
	if (msg->first < packetIDRecived) {
	myPlayer->resending.erase(msg->first);
	}
	}*/

}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(500, 500), "Traffic Game");
	c.restart();

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

		for (int8_t i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (int8_t j = 0; j < NUMBER_ROWS_COLUMNS; j++)
			{
				sf::RectangleShape rectBlanco(sf::Vector2f(SIZE_CELL, SIZE_CELL));
				sf::Color grey = sf::Color(49, 51, 53);
				rectBlanco.setFillColor(grey);
				//rectBlanco.setOutlineColor(sf::Color::Green);
				//rectBlanco.setOutlineThickness(2.f);s
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

		for (std::map<int8_t, Position>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
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
	packet << "NEWCONNECTION"; //poner packetID
	status = socket.send(packet, serverIP, serverPORT); //fer resend?
	if (status == sf::Socket::Error) std::cout << "Error." << std::endl;
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Server disconnected" << std::endl;
		socket.unbind();
	}

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
	system("pause");

	return 0;
}



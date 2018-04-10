//TALLER 6 - ANNA PONCE I MARC SEGARRA
/////////////////////////////////////
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
#define RESEND 200

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
int state = 1;
sf::UdpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
int idPacket = 1;
bool connected = false;
sf::Clock clockResend;

struct Position {
	int x;
	int y;
};

struct Player
{
	int id;
	Position position;
	std::map<int, sf::Packet> resending;
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

void Resend() {

	if (clockResend.getElapsedTime().asMilliseconds() > RESEND) { //cada cert temps torno ha enviar
		for (std::map<int, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {

			status = socket.send(msg->second, "localhost", PORT);

			if (status == sf::Socket::Error)
				std::cout << "Error sending the message. Client to Server." << std::endl;
		}
		clockResend.restart();
	}
}


void SendDueReceived(std::string cmd, int idPacketReceived) {
	//Enviamos a una IP:Puerto concreto, porque el socket no está vinculado
	//a ningún otro socket en exclusiva
	sf::Packet packet;

	if (cmd == "ACK_PING") {
		packet << "ACK_PING" << idPacketReceived << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;

	}
	if (cmd == "ACK") {
		packet << "ACK" << idPacketReceived << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;

	}
	if (cmd == "ACK_WELCOME") {
		packet << "ACK_WELCOME" << idPacketReceived << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;

	}
}

void SendDueClient(std::string cmd) {
	sf::Packet packet;
	if (cmd == "DISCONNECTION") {
		packet << "DISCONNECTION" << idPacket++ << myPlayer->id;
		status = socket.send(packet, "localhost", PORT);
		if (status == sf::Socket::Error) std::cout << "Error" << std::endl;
	}
	if (cmd == "NEWCONNECTION") {
		packet << "NEWCONNECTION" << idPacket++ <<-1; //encara no te id
		//status = socket.send(packet, "localhost", PORT);
		myPlayer->resending.insert(std::make_pair(idPacket, packet));
	}
}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	std::string cmd;
	int clientId;
	int idPacktReceived;
	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		packet >> cmd >> idPacktReceived;

		if (cmd == "PING") { // no te id del oponent quan reb el packet
			SendDueReceived("ACK_PING", idPacktReceived);
		}
		else if (cmd == "ACK") {  // no te id del oponent quan reb el packet
			//confirmacio com tal a rebut el missatge el servidor
		}
		else {
			packet >> clientId;

			if (cmd == "CONNECTION") {
				Position pos;
				packet >> pos.x >> pos.y;
				std::cout << "A new opponent connected. ID: " << clientId << " Position: " << pos.x << ", " << pos.y << std::endl;
				opponents.insert(std::make_pair(clientId, pos));

				SendDueReceived("ACK", idPacktReceived); //resposta al servidor com tal ha rebut el packet el client
			}
			else if (cmd == "DISCONNECTION") {
				std::cout << "An opponent disconnected. ID: " << clientId << std::endl;
				opponents.erase(clientId);

				SendDueReceived("ACK", idPacktReceived); //resposta al servidor com tal ha rebut el packet el client
			}
			else if (cmd == "POSITION") { //update all positions
				Position pos;
				packet >> pos.x >> pos.y;

				if (clientId == myPlayer->id) {
					myPlayer->position = pos; //soc jo
				}
				else if (opponents.find(clientId) == opponents.end()) {
					//encara no esta a la llista
					opponents.insert(std::make_pair(clientId, pos));
				}
				else {
					//esta a la llista i no soc jo
					opponents.find(clientId)->second = pos;
				}

				SendDueReceived("ACK", idPacktReceived); //resposta al servidor com tal ha rebut el packet el client
			}
			else if (cmd == "WELCOME") {
				packet >> myPlayer->position.x >> myPlayer->position.y;
				myPlayer->id = clientId;
				std::cout << "WELCOME! " << "Client ID: " << clientId << " Initial Position: " << myPlayer->position.x << ", " << myPlayer->position.y << std::endl;
				SendDueReceived("ACK_WELCOME", idPacktReceived); //resposta al servidor com tal ha rebut el packet el client
				connected = true;

				
			}
		
		}
	}
}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(500, 500), "Traffic Game");
	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();
		Resend();
		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				SendDueClient("DISCONNECTION");
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
	SendDueClient("NEWCONNECTION");
	socket.setBlocking(false);

	do {
		//esperar fins rebre confirmacio del server com tal estic connectat
		ReceiveData();
		Resend();
	} while (!connected);


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
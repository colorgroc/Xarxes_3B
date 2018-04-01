//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#define MAX_MENSAJES 25

#define RECEIVED 1
#define WRITED 2
#define CONNECTION 3
#define PORT 50000

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;
int state = 1;

sf::UdpSocket socket;
std::vector<sf::UdpSocket*> aSock;

int clientID;
sf::Socket::Status status;
std::mutex myMutex;

struct Position {
	int x;
	int y;
}position;


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
		packet >> msg >> clientID >> position.x >> position.y;
		std::cout << msg << "Client ID: " << clientID << " Initial Position: " << position.x << ", " << position.y << std::endl;
	}
}

int main()
{
	std::cout << "Estableciendo conexion con server... \n";
	Send();
	Receive();

		//socket.disconnect();
	system("pause");

	return 0;
}
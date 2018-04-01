//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 4
#define MAX_MENSAJES 25
#define NEW_CONNECTION 1
#define DISCONNECTED 2
#define PORT 50000

using PacketID = sf::Int8;
//using ClientID = int;
//using PortNumber = unsigned short;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } bingo;

bool online;

struct Position {
	int x;
	int y;
};

sf::Socket::Status status;
std::mutex myMutex;
int clientID = 1;
std::string textoAEnviar = "";

//sf::TcpListener listener;
std::vector<sf::UdpSocket*> clients;
sf::UdpSocket socket;
//sf::SocketSelector selector;


void NotifyAllClients(int option, sf::UdpSocket *newclient) {

	//cuando se conecte un nuevo cliente
	if (option == NEW_CONNECTION) {
		for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			
		}
	}
	//cuando se desconecta un cliente
	else if (option == DISCONNECTED) {
		for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			
		}
	}

}

void SendToAllClients(sf::UdpSocket *fromclient, std::string msg) {

	for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		
	}
}



void ControlServidor()
{
	
	// bind the socket to a port
	status = socket.bind(PORT);
	if(status != sf::Socket::Done)
	{
		socket.unbind();
		exit(0);
	}
	std::cout << "Server is listening to port " << PORT << ", waiting for clients " << std::endl;

	

}

void ReceiveData() {
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	status = socket.receive(packet, senderIP, senderPort);
	if (status == sf::Socket::Done) {
		std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
		Position pos;
		pos.x = std::rand() % 500 + 1;
		pos.y = std::rand() % 500 + 1;
		packet << "WELCOME! " << clientID << pos.x << pos.y;
		status = socket.send(packet, senderIP, senderPort);
		if (status != sf::Socket::Done) std::cout << "Error sending the message." << std::endl;
		clients.push_back(&socket);
		clientID++;
	} //else if(status == sf::Socket::Disconnected){
		//eliminar client corresponent...fer bucle 
		//clients.erase(clients.begin, )
	//}
}

int main()
{
	ControlServidor();
	do {
		ReceiveData();
	} while (clients.size() < 4);

	system("pause");
	return 0;
}
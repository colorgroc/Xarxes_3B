//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 4

#define CONNECTION 1
#define DISCONNECTION 2
#define MOVE_UP 3
#define MOVE_DOWN 4
#define MOVE_RIGHT 5
#define MOVE_LEFT 6
#define PORT 50000

using PacketID = sf::Int8;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } game;
//struct Action {
//	int _DISCONNECTION = 1;
//	int _MOVE_UP = 2;
//	int _MOVE_DOWN = 3;
//	int _MOVE_RIGHT = 4;
//	int _MOVE_LEFT = 5;
//}action;

bool online = true;

struct Position {
	int x;
	int y;
};
struct Client {
	//int ID;
	Position pos;
	sf::IpAddress IP;
	unsigned short port;
	//sf::UdpSocket* socket;

};
sf::Socket::Status status;
std::mutex myMutex;
int clientID = 1;
std::string textoAEnviar = "";

//std::vector<Client*> clients;
std::map<int , Client*> clients;
sf::UdpSocket socket;


void NotifyAllClients(int option, int id) {
	
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != id) {
			sf::Packet p;
			if (option == CONNECTION) {
				p << CONNECTION << id << clients.find(id)->second->pos.x << clients.find(id)->second->pos.y;		
			}
			else if (option == DISCONNECTION) {
				p << DISCONNECTION << id << clients.find(id)->second->pos.x << clients.find(id)->second->pos.y;
			}
			socket.send(p, it->second->IP, it->second->port); //controlar errors
		}
		
	}
}

void Receive(int _action, int id, sf::IpAddress senderIP, unsigned short senderPort, sf::Packet p) {
	if (_action == DISCONNECTION) {
		NotifyAllClients(DISCONNECTION, id);
		clients.erase(id);
	}
	else if (_action == MOVE_UP) {

	}
	else if (_action == MOVE_DOWN) {

	}
	else if (_action == MOVE_RIGHT) {

	}
	else if (_action == MOVE_LEFT) {

	}
}


void SendToAllClients(sf::UdpSocket *fromclient, std::string msg) {

	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
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

void ReceiveDataInit() {
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;

	status = socket.receive(packet, senderIP, senderPort);
	if (status == sf::Socket::Done) {
		std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
		Position pos;
		pos.x = std::rand() % 25 + 1;
		pos.y = std::rand() % 25 + 1;
		packet << "WELCOME! " << clientID << pos.x << pos.y;

		status = socket.send(packet, senderIP, senderPort);
		if (status != sf::Socket::Done) std::cout << "Error sending the message." << std::endl;
		//clients.push_back(new Client{ clientID, pos, senderIP, senderPort});//&socket});
		clients.insert(std::make_pair(clientID, new Client{pos, senderIP, senderPort }));
		NotifyAllClients(CONNECTION, clientID);
		clientID++;
	} 
	
}

void ReceiveData() {
	//HACER MUTEX/THREAD O NONBLOCKING
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int id;
	int action;
	status = socket.receive(packet, senderIP, senderPort);
	
	if (status == sf::Socket::Done) {
		
		//Position pos;
		packet >> action >> id; //>> pos.x, pos.y;
		Receive(action, id, senderIP, senderPort, packet);

	}
	else if (status == sf::Socket::Disconnected) {
		for (int i = 0; i < clients.size(); i++) {

		}
	}
}

int main()
{
	socket.setBlocking(false);
	ControlServidor();
	do {
		ReceiveDataInit();
	} while (clients.size() < MAX_CLIENTS);
	do {
		ReceiveData();
		if (clients.size() <= 0) online = false;
	} while (online);
	clients.clear();
	socket.unbind();
	system("pause");
	return 0;
}
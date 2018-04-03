//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 3

#define PORT 50000

using Comando = sf::Int8;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } comandos;

bool online = true;

struct Position {
	int x;
	int y;
};
struct Client {
	int8_t clientID;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	std::map<int, sf::Packet> resending;
};


sf::Socket::Status status;
std::mutex myMutex;
int clientID = 1;
std::string textoAEnviar = "";
int8_t packetID = 1;
std::map<int , Client> clients;
sf::UdpSocket socket;
sf::Clock c;
/*c.restart();
while (c.getElapsedTime().asMilliseconds() >= PING);*/

void Resend() {

	//reenviar

	//ferho en un thread q es reenvii cada PING segons en el main no?
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it){
		for (std::map<int, sf::Packet>::iterator it2 = it->second.resending.begin(); it2 != it->second.resending.end(); ++it) {
			status = socket.send(it2->second, it->second.ip, it->second.port);
			if (status != sf::Socket::Done)
				std::cout << "Error sending the message." << std::endl;
		}
	}


	//enviar resposta .--> NOOO CAL GUARDAR-HO. QUAN REP, BORRARLO DEL MAP. NO SHAN DE GUARDAR ELS ACK
}


void NotifyOtherClients(std::string cmd, int id) {
	
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != id) {
			sf::Packet p;
			if (cmd == "CONNECTION") {
				p << packetID << "CONNECTION" << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;		
			}
			else if (cmd == "DISCONNECTION") {
				p << packetID << "DISCONNECTION" << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
			}
			clients.find(id)->second.resending.insert(std::make_pair(packetID, p));
			packetID++;

			/*clients[id]->resending.insert(std::make_pair(packetID, Resend{ p, it->second.ip, it->second.port }));
			packetID++;*/
			//socket.send(p, it->second.ip, it->second.port); //controlar errors
		}
	}
}

void Receive(std::string cmd, int id, sf::IpAddress senderIP, unsigned short senderPort, sf::Packet p) {
	if (cmd == "DISCONNECTION") {
		NotifyOtherClients("DISCONNECTION", id);
		clients.erase(id);
	}
	else if (cmd == "MOVE_UP") {

	}
	else if (cmd == "MOVE_DOWN") {

	}
	else if (cmd == "MOVE_RIGHT") {

	}
	else if (cmd == "MOVE_LEFT") {

	}
}


void SendToAllClients(std::string cmd) {

	for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
	{
		for (std::map<int, Client>::iterator otherClients = clients.begin(); otherClients != clients.end(); ++otherClients)
		{
			sf::Packet p;
			if (cmd == "POSITION") {
				p << packetID << cmd << otherClients->first << otherClients->second.pos.x << otherClients->second.pos.y;
				otherClients->second.resending.insert(std::make_pair(packetID, p));
				packetID++;
				//socket.send(p, clientToSend->second.ip, clientToSend->second.port); //controlar errors
			}
		}
		
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

	do {
		sf::Packet packet;
		sf::IpAddress senderIP;
		unsigned short senderPort;
		std::string command;
		status = socket.receive(packet, senderIP, senderPort);
	
		if (status == sf::Socket::Done) {
			packet >> command;
			if (command == "NEW_CONNECTION") {
				std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
				Position pos;
				srand(time(NULL));
				pos.x = std::rand() % 25;
				pos.y = std::rand() % 25;
				packet.clear();
				packet << "WELCOME" << clientID << pos.x << pos.y;
				clients.insert(std::make_pair(clientID, Client{ clientID, pos, senderIP, senderPort }));

				clients.find(clientID)->second.resending.insert(std::make_pair(packetID, packet));
				packetID++;

				//status = socket.send(packet, senderIP, senderPort);
				//if (status == sf::Socket::Done) {
					
					NotifyOtherClients("CONNECTION", clientID);
					SendToAllClients("POSITION");
					clientID++;
				//}
				/*else {
					std::cout << "Error sending the message." << std::endl;
				}*/
			}
		}
	} while (clients.size() != MAX_CLIENTS);

	socket.setBlocking(false);
}


void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int id;
	std::string cmd;
	status = socket.receive(packet, senderIP, senderPort);
	
	if (status == sf::Socket::Done) {	
		//Position pos;
		packet >> cmd >> id; //>> pos.x, pos.y;
		Receive(cmd, id, senderIP, senderPort, packet);
	}
	else if (status == sf::Socket::Disconnected) {
		for (int i = 0; i < clients.size(); i++) {

		}
	}
}

int main()
{
	ControlServidor();
	sf::Thread(&ReciveAck);
	do {
		//ReceiveData();
		if (clients.size() <= 0) online = false;
	} while (online);

	clients.clear();
	socket.unbind();
	system("pause");
	return 0;
}
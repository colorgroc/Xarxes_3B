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
#define PING 1000
#define CONTROL_PING 10000
#define PORT 50000

using PacketID = sf::Int8;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } game;

bool online = true;

struct Position {
	int x;
	int y;
};
struct Client {
	int id;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	std::map<int, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
};

sf::Socket::Status status;
std::mutex myMutex;
int clientID = 1;

std::map<int, Client> clients;
sf::UdpSocket socket;

sf::Clock clockPing;
bool once = false;
int packetID = 1;

void NotifyOtherClients(std::string cmd, int id) {

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != id) {
			sf::Packet p;
			if (cmd == "CONNECTION") {
				p << "CONNECTION" << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
			}
			else if (cmd == "DISCONNECTION") {
				p << "DISCONNECTION" << id;
			}
			socket.send(p, it->second.ip, it->second.port); //controlar errors
		}
	}
}

void SendToAllClients(std::string cmd) {

	if (cmd == "POSITION") {
		for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			for (std::map<int, Client>::iterator otherClients = clients.begin(); otherClients != clients.end(); ++otherClients)
			{
				sf::Packet p;
				p << cmd << otherClients->first << otherClients->second.pos.x << otherClients->second.pos.y;
				socket.send(p, clientToSend->second.ip, clientToSend->second.port); //controlar errors
			}

		}
	}

	if (cmd == "PING") {
		for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet p;
			p << cmd;
			socket.send(p, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}

}


void ManageReveivedData(std::string cmd, int id, sf::IpAddress senderIP, unsigned short senderPort, sf::Packet p) {
	if (cmd == "DISCONNECTION") {
		NotifyOtherClients("DISCONNECTION", id);
		clients.erase(id);
	}

	//rebem resposta del ping i per tant encara esta conectat
	//fem reset del seu rellotge intern
	if (cmd == "ACK_PING") {
			clients.find(id)->second.timeElapsedLastPing.restart();
	}
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
		ManageReveivedData(cmd, id, senderIP, senderPort, packet);
	}
}


void ControlServidor()
{
	// bind the socket to a port
	status = socket.bind(PORT);
	if (status != sf::Socket::Done)
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
			if (command == "NEWCONNECTION") {
				std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
				Position pos;
				srand(time(NULL));
				pos.x = std::rand() % 25;
				pos.y = std::rand() % 25;
				packet.clear();
				packet << "WELCOME" << clientID << pos.x << pos.y;

				status = socket.send(packet, senderIP, senderPort);
				if (status == sf::Socket::Done) {
					clients.insert(std::make_pair(clientID, Client{clientID, pos, senderIP, senderPort }));
					NotifyOtherClients("CONNECTION", clientID);
					SendToAllClients("POSITION");
					clientID++;
				}
				else {
					std::cout << "Error sending the message." << std::endl;
				}
			}
		}
	} while (clients.size() != MAX_CLIENTS);

	socket.setBlocking(false);
}


void ManagePing() {
	
	if (!once) {
		//poso a zero tots els temps
		for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			it->second.timeElapsedLastPing.restart();
		}
		//a zero el rellotge
		clockPing.restart();
		once = true;
	}
	

	//cada certa quantiat de temps enviar missatge ping
	if (clockPing.getElapsedTime().asMilliseconds() > PING) {
		SendToAllClients("PING");
		clockPing.restart();

		//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
		//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
		for (std::map<int, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
			if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
				NotifyOtherClients("DISCONNECTION", clientes->first);
				clients.erase(clientes->first);
			}
		}
	}
}

int main()
{
	ControlServidor();
	
	do {
		ManagePing();
		ReceiveData();
	} while (clients.size() >= 0);

	clients.clear();
	socket.unbind();
	system("exit");
	return 0;
}
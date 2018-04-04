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
#define PING 500
#define CONTROL_PING 8000
#define PORT 50000

using Comando = sf::Int8;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } comandos;

bool online = true;

struct Position {
	int x;
	int y;
};
struct Client {
	int clientID; //nose si cal guardar la seva id si ja la guardem en la key dl map --> guardar nickname
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	std::map<int, sf::Packet> resending;
	sf::Clock timePastPING;
};

std::map<int, sf::Packet> connectionResending;
sf::Socket::Status status;
std::mutex myMutex;
int clientID = 1;
std::string textoAEnviar = "";
int packetID = 1;
std::map<int, Client> clients;
sf::UdpSocket socket;
sf::Clock c;
/*c.restart();
while (c.getElapsedTime().asMilliseconds() >= PING);*/

void ConnectionResend() {
	for (std::map<int, sf::Packet>::iterator msg = connectionResending.begin(); msg != connectionResending.end(); ++msg) {
		status = socket.send(msg->second, clients[msg->first].ip, clients[msg->first].port);
		if (status == sf::Socket::Error)
			std::cout << "Error sending the message." << std::endl;
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Error sending the message. Client disconnected." << std::endl;
			clients.erase(msg->first); //msg->first es la id del client
		}
	}
}


void Resend() {
	//posar mutex??
	for (std::map<int, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		for (std::map<int, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			status = socket.send(msg->second, clientes->second.ip, clientes->second.port);
			if (status == sf::Socket::Error)
				std::cout << "Error sending the message." << std::endl;
			else if (status == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clients.erase(clientes);
			}
		}
	}
}

void NotifyOtherClients(std::string cmd, int id) {

	for(std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->first != id) {
			sf::Packet packet;
			int iden = id + 10;
			if (cmd == "CONNECTION") {
				packet << packetID << "CONNECTION" << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
				connectionResending.insert(std::make_pair(iden, packet));
			}
			else if (cmd == "DISCONNECTION") {
				packet << packetID << "DISCONNECTION" << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
				clients.find(id)->second.resending.insert(std::make_pair(packetID, packet));
				packetID++;
			}

		}
	}
}


void SendToAllClients(std::string cmd) {

	for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
	{
		for (std::map<int, Client>::iterator otherClients = clients.begin(); otherClients != clients.end(); ++otherClients)
		{
			if (otherClients->first != clientToSend->first) { //no tho enviis a tu mateix
				sf::Packet packet;
				if (cmd == "POSITION") {
					packet << packetID << cmd << otherClients->first << otherClients->second.pos.x << otherClients->second.pos.y;
					otherClients->second.resending.insert(std::make_pair(packetID, packet));
					packetID++;
					//socket.send(p, clientToSend->second.ip, clientToSend->second.port); //controlar errors
				}
			}
		}

	}
}

void ConnectionReceive() {
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int cID;
	std::string cmd;
	status = socket.receive(packet, senderIP, senderPort);
	if (cmd == "WELCOME_ACK") {
		if (connectionResending.find(cID) != connectionResending.end())
			connectionResending.erase(cID);
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
	sf::Clock t;
	t.restart();
	do {
		sf::Packet packet;
		sf::IpAddress senderIP;
		unsigned short senderPort;
		std::string cmd;
		status = socket.receive(packet, senderIP, senderPort);

		if (status == sf::Socket::Done) {
			packet >> cmd;
			if (cmd == "NEW_CONNECTION") {
				std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
				Position pos;
				srand(time(NULL));
				pos.x = std::rand() % 25;
				pos.y = std::rand() % 25;
				packet.clear();
				packet << "WELCOME" << clientID << pos.x << pos.y;
				clients.insert(std::make_pair(clientID, Client{ clientID, pos, senderIP, senderPort }));
				connectionResending.insert(std::make_pair(clientID, packet));
				if (t.getElapsedTime().asMilliseconds() >= PING) {
					ConnectionResend();
					t.restart();
				}
				NotifyOtherClients("CONNECTION", clientID);
				ConnectionReceive();
				//clients.find(clientID)->second.resending.insert(std::make_pair(packetID, packet));
				//packetID++;
				
				clientID++;
			}
		}
	} while (clients.size() != MAX_CLIENTS);

	SendToAllClients("POSITION");
	socket.setBlocking(false);
}



void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int cID;
	int pID;
	std::string cmd;
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {
		//Position pos;
		packet >> pID >> cmd >> cID;
		if (cmd == "ACK") {
			if (clients.find(cID) != clients.end() && clients[cID].resending.find(pID) != clients[cID].resending.end())
				clients[cID].resending.erase(pID);
		}else if (cmd == "PING") {
			clients[cID].timePastPING.restart();
		}
		/*else if (cmd == "WELCOME_ACK") {
		if (connectionResending.find(cID) != connectionResending.end())
		connectionResending.erase(cID);
		}*/
		else {
			//pillar pos? packet >> pos.x >> pos.y; ??
			//posar aqui les accions

			//enviar ACK
			packet.clear();
			packet << pID << "ACK" << cID;
			clients[cID].resending.insert(std::make_pair(pID, packet));
		}
	}
}

void RestartTimeClients() {
	for (std::map<int, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		clientes->second.timePastPING.restart();
	}
}

void ControlPingClients() {
	for (std::map<int, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		if (clientes->second.timePastPING.getElapsedTime().asMilliseconds() >= CONTROL_PING) {
			NotifyOtherClients("DISCONNECTION", clientes->first);
			clients.erase(clientes->first);
		}
	}
}

int main()
{
	ControlServidor();
	RestartTimeClients();
	c.restart();

	do {
		ControlPingClients();
		if (c.getElapsedTime().asMilliseconds() >= PING) {
			Resend();
			c.restart();
		}
		ReceiveData();
		if (clients.size() <= 0) online = false;
	} while (online);

	clients.clear();
	socket.unbind();
	system("pause");
	return 0;
}
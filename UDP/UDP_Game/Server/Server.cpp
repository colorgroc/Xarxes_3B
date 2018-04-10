//TALLER 6 - ANNA PONCE I MARC SEGARRA
//////////////////////////////////////
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 3
#define PING 1000
#define RESEND 200
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
sf::Clock clockResend;
int packetID = 1;

Position lastPosCreated;

void Resend() {

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		for (std::map<int, sf::Packet>::iterator msg = it->second.resending.begin(); msg != it->second.resending.end(); ++msg) {
			status = socket.send(msg->second, it->second.ip, it->second.port);
			if (status == sf::Socket::Error)
				std::cout << "Error sending the message. Server to Client." << std::endl;
		}
	}

}

void NotifyOtherClients(std::string cmd, int id) {

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != id) {
			sf::Packet packet;
			if (cmd == "CONNECTION") {
				packet << "CONNECTION" << packetID++ << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
			}
			else if (cmd == "DISCONNECTION") {
				packet << "DISCONNECTION" << packetID++ << id;
			}
			clients.find(id)->second.resending.insert(std::make_pair(packetID, packet));
		}
	}
}

void SendToAllClients(std::string cmd) {

	if (cmd == "POSITION") {
		for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			for (std::map<int, Client>::iterator otherClients = clients.begin(); otherClients != clients.end(); ++otherClients)
			{
				sf::Packet packet;
				packet << cmd << packetID << otherClients->first << otherClients->second.pos.x << otherClients->second.pos.y;
				clientToSend->second.resending.insert(std::make_pair(packetID, packet));
			}
			packetID++;
		}
	}

	if (cmd == "PING") {
		for (std::map<int, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd, 0; //tots els pings tenen el mateix idPacket, zero
			socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}

}

void SendDueReceivedToOneClient(std::string cmd, int idPacketReceived, int id, sf::IpAddress senderIP, unsigned short senderPort) {

	if (cmd == "NEWCONNECTION") {
		sf::Packet packet;
		Position pos;
		srand(time(NULL));
		pos.x = std::rand() % 25;
		pos.y = std::rand() % 25;
		lastPosCreated = pos;
		packet << "WELCOME" << packetID++ << clientID << pos.x << pos.y;

		clients.find(id)->second.resending.insert(std::make_pair(packetID, packet));

	}
	//hi han missatges que tindran que respondre al client amb un ack
}


void ManageReveivedData(std::string cmd, int idPacketReceived, int id, sf::IpAddress senderIP, unsigned short senderPort, sf::Packet p) {
	if (cmd == "DISCONNECTION") {
		NotifyOtherClients("DISCONNECTION", id);
		clients.erase(id);
	}

	//rebem resposta del ping i per tant encara esta conectat
	//fem reset del seu rellotge intern
	if (cmd == "ACK_PING") {
		clients.find(id)->second.timeElapsedLastPing.restart();
	}

	if (cmd == "ACK") {
		if (clients.find(id)->second.resending.find(idPacketReceived) != clients.find(id)->second.resending.end()) {
			clients.find(id)->second.resending.erase(idPacketReceived);
		}
	}
	if (cmd == "NEWCONNECTION") {
		std::cout << "Connection with client " << clientID << " from PORT " << senderPort << std::endl;
		SendDueReceivedToOneClient("NEWCONNECTION", idPacketReceived, clientID, senderIP, senderPort);
	}
	if (cmd == "ACK_WELCOME") {
		clients.insert(std::make_pair(clientID, Client{ clientID, lastPosCreated, senderIP, senderPort }));
		NotifyOtherClients("CONNECTION", clientID);
		SendToAllClients("POSITION");
		clientID++;
		clients.find(id)->second.resending.erase(idPacketReceived);
	}

}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int id;
	std::string cmd;
	int idPacketReceived;
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {
		packet >> cmd >> idPacketReceived >> id;
		ManageReveivedData(cmd, idPacketReceived, id, senderIP, senderPort, packet);
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
	clockResend.restart();

	do {
	
		ReceiveData();
		if (clockResend.getElapsedTime().asMilliseconds() > RESEND) { //cada cert temps torno ha enviar
			Resend();
			clockResend.restart();
		}
		ManagePing();

	} while (clients.size() >= 0);

	clients.clear();
	socket.unbind();
	system("exit");
	return 0;
}
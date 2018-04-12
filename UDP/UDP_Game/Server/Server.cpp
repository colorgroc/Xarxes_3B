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
#define SENDING_PING 500
#define _PING 1000
#define CONTROL_PING 5000
#define PORT 50000

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

bool online = true;


struct Position {
	sf::Int8 x;
	sf::Int8 y;
};

struct Client {
	sf::Int8 id;
	std::string nickname;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	bool connected;
	std::map<sf::Int8, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
};

sf::Socket::Status status;
std::mutex myMutex;
sf::Int8 clientID = 1;

std::map<sf::Int8, Client> clients;
sf::UdpSocket socket;

sf::Clock clockPing, clockSend;
bool once = false;
sf::Int8 packetID = 1;

/*sf::Packet& operator <<(sf::Packet& Packet, const Position& pos)
{
	return Packet << pos.x << pos.y;
}

sf::Packet& operator >>(sf::Packet& Packet, Position& pos)
{
	return Packet >> pos.x >> pos.y;
}*/

void Resend() {
	//posar mutex??
	for (std::map<sf::Int8, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		for (std::map<sf::Int8, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			status = socket.send(msg->second, clientes->second.ip, clientes->second.port);
			if (status == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			else if (status == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clients.erase(clientes);
			}
			/*else {
				std::string cmd;
				sf::Int8 pID;
				msg->second >> cmd >> pID;
				std::cout << "Mensage enviado. Cliente: " << std::to_string(clientes->first) << " ID Packet: " << std::to_string(pID) << "Comando: " << cmd << std::endl;
			}*/
		}
	}
}



void SendToAllClients(sf::Int8 cmd) {

	if (cmd == PING) {
		for (std::map<sf::Int8, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}
}

void NotifyOtherClients(sf::Int8 cmd, sf::Int8 cID) {
	if (cmd == NEW_CONNECTION) {
		if (clients.find(cID) != clients.end()) {
			for (std::map<sf::Int8, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetID << cID << clients.find(cID)->second.pos.x << clients.find(cID)->second.pos.y;
					it->second.resending.insert(std::make_pair(packetID, packet));
				}
			} packetID++;
		}
	}
	else if (cmd == DISCONNECTION) {
		//if (clients.find(cID) != clients.end()) {
			for (std::map<sf::Int8, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				sf::Packet packet;
				packet << cmd;
				if (it->first != cID) {
					packet << packetID << cID;
					it->second.resending.insert(std::make_pair(packetID, packet));
				}
			}packetID++;
		//}
	}
}


void ManageReveivedData(sf::Int8 cmd, sf::Int8 cID, sf::Int8 pID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname, Position trypos) {
	

	if (cmd == ACK_PING) {
		if(clients.find(cID) != clients.end())
		clients.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == HELLO && clients.size() < MAX_CLIENTS) {
		//comprobar si el client ja esta connectat. Anteriorment comprovat amb el port pero es pot donar el cas d q es repeteixi el port
		bool alreadyConnected = false;
		for (std::map<sf::Int8, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->second.nickname == nickname) {
				alreadyConnected = true;
			}
		}

		Position pos;
		sf::Int8 numOfOpponents = clients.size();
		srand(time(NULL));
		pos.x = std::rand() % 25;
		pos.y = std::rand() % 25;
		sf::Packet packet;
		packet.clear();
		if (!alreadyConnected) {
			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			packet << ACK_HELLO << pID << clientID << pos.x << pos.y << numOfOpponents;
			if (numOfOpponents > 0) {
				//inserim al packet la ID i la pos de cada oponent
				for (std::map<sf::Int8, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
					packet << it->second.id << it->second.pos.x << it->second.pos.y;
				}
			}
			clients.insert(std::make_pair(clientID, Client{ clientID, nickname, pos, senderIP, senderPort, true }));
			NotifyOtherClients(NEW_CONNECTION, clientID);
			//clients[clientID].resending.insert(std::make_pair(packetID, packet));
			clientID++;		
		}
		status = socket.send(packet, senderIP, senderPort);
		if (status != sf::Socket::Done) {
			std::cout << "Error sending ACK_HELLO to client " << std::to_string(clientID-1) << std::endl;
		}//else std::cout << "Sent ACK_HELLO to client " << std::to_string(clientID - 1) << std::endl;
		packet.clear();
	}
	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION) {
		if (clients.find(cID) != clients.end() && clients[cID].resending.find(pID) != clients[cID].resending.end()) {
			clients[cID].resending.erase(pID);
		}
	}
	else if (cmd == TRY_POSITION) {
		if (trypos.x != LEFT_LIMIT && trypos.x != RIGHT_LIMIT - 1 && trypos.y != TOP_LIMIT && trypos.y != LOW_LIMIT - 1) {
			clients.find(cID)->second.pos = trypos; //si esta dintre del mapa mou
			std::cout << "new position " << std::to_string(clients.find(cID)->second.pos.x) << " " << std::to_string(clients.find(cID)->second.pos.y) <<std::endl;
		}
		
	}

}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	sf::Int8 IDClient = 0;
	sf::Int8 packetIDRecived = 0;
	sf::Int8 cmd;
	std::string nickname = "";
	Position trypos{ 0, 0 };
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {	
		packet >> cmd >> packetIDRecived;
		if (cmd == HELLO) {
			packet >> nickname;
		}
		else {
			packet >> IDClient;

			if (cmd == TRY_POSITION) {
				packet >> trypos.x >> trypos.y;
			}
		}
			
		ManageReveivedData(cmd, IDClient, packetIDRecived, senderIP, senderPort, nickname, trypos);
	}
	packet.clear();
}


void ControlServidor()
{
	// bind the socket to a port
	status = socket.bind(PORT);
	socket.setBlocking(false);
	if (status != sf::Socket::Done)
	{
		socket.unbind();
		exit(0);
	}
	std::cout << "Server is listening to port " << PORT << ", waiting for clients " << std::endl;

}


void ManagePing() {

	if (!once) {
		//poso a zero tots els temps
		for (std::map<sf::Int8, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			it->second.timeElapsedLastPing.restart();
		}
		//a zero el rellotge
		clockPing.restart();
		once = true;
	}


	//cada certa quantiat de temps enviar missatge ping
	if (clockPing.getElapsedTime().asMilliseconds() > _PING) {
		SendToAllClients(PING);
		clockPing.restart();
	}
	//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	for (std::map<sf::Int8, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			NotifyOtherClients(DISCONNECTION, clientes->first);
			//clients.erase(clientes->first);
			clientes->second.connected = false;
		}
	}

	for (sf::Int8 i = 0; i < clients.size(); i++) {
		if (!clients[i].connected) {
			clients.erase(clients[i].id);
		}
	}
}

int main()
{
	ControlServidor();
	clockSend.restart();
	//SI EL Q ES VOL ES Q NO SURTIN LES PESTANYES DE JUGAR FINS Q TOTS NO ESTIGUIN CONNECTATS, ALESHORES FER EL RECEIVE DEL WELCOME I EL SEND COM ESTAVA EN ELS ANTERIORS COMMITS
	do {
		ReceiveData();
		ManagePing();
		//cada certa quantiat de temps enviar missatge ping
		if (clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			clockSend.restart();
		}
		
		
	} while (clients.size() >= 0);// && clients.size() <= MAX_CLIENTS);

	clients.clear();
	socket.unbind();
	system("exit");
	return 0;
}

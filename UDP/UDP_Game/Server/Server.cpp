//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 2
#define SENDING_PING 500
#define PING 5000
#define CONTROL_PING 10000
#define PORT 50000

//using PacketID = sf::Int8;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } game;

bool online = true;

struct Position {
	int8_t x;
	int8_t y;
};
struct Client {
	int8_t id;
	Position pos;
	sf::IpAddress ip;
	unsigned short port;
	bool connected;
	std::map<int8_t, sf::Packet> resending;
	sf::Clock timeElapsedLastPing;
};

sf::Socket::Status status;
std::mutex myMutex;
int8_t clientID = 1;

std::map<int8_t, Client> clients;
sf::UdpSocket socket;

sf::Clock clockPing, clockSend;
bool once = false;
int8_t packetID = 1;


void Resend() {
	//posar mutex??
	for (std::map<int8_t, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		for (std::map<int8_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			status = socket.send(msg->second, clientes->second.ip, clientes->second.port);
			if (status == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			else if (status == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clients.erase(clientes);
			}
			else {
				std::string cmd;
				int8_t pID;
				msg->second >> cmd >> pID;
				std::cout << "Mensage enviado. Cliente: " << std::to_string(clientes->first) << " ID Packet: " << std::to_string(pID) << "Comando: " << cmd << std::endl;
			}
		}
	}
}

void NotifyOtherClients(std::string cmd, int8_t id) {

	for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->first != id) {
			sf::Packet packet;
			if (cmd == "CONNECTION") {
				packet << cmd << packetID << id << clients.find(id)->second.pos.x << clients.find(id)->second.pos.y;
			}
			else if (cmd == "DISCONNECTION") {
				packet << cmd << packetID << id;
			}
			//socket.send(p, it->second.ip, it->second.port); //controlar errors
			//if (clients.find(id)->second.resending.find(packetID) == clients.find(id)->second.resending.end()) {
				clients.find(id)->second.resending.insert(std::make_pair(packetID, packet));
				packetID++;
			//}
		}
	}
}

void SendToAllClients(std::string cmd) {

	if (cmd == "POSITION") {
		for (std::map<int8_t, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			for (std::map<int8_t, Client>::iterator otherClients = clients.begin(); otherClients != clients.end(); ++otherClients)
			{
				sf::Packet packet;
				packet << cmd << packetID << otherClients->first << otherClients->second.pos.x << otherClients->second.pos.y;
				//if (clientToSend->second.resending.find(packetID) == clientToSend->second.resending.end()) {
					clientToSend->second.resending.insert(std::make_pair(packetID, packet));
					//socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
				//}

			}

		}packetID++;
	}

	if (cmd == "PING") {
		for (std::map<int8_t, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}

}


void ManageReveivedData(std::string cmd, int8_t cID, int8_t pID, sf::IpAddress senderIP, unsigned short senderPort, sf::Packet packet) {


	//mirar si hem rebut algun packet amb un id superior
	/*for (std::map<int, sf::Packet>::iterator msg = clients.find(cID)->second.resending.begin(); msg != clients.find(cID)->second.resending.end(); ++msg) {
	if (msg->first < pID) {
	clients.find(cID)->second.resending.erase(msg->first);
	}
	}*/

	/*if (cmd == "ACK") {
		if (clients.find(cID)->second.resending.find(pID) != clients.find(cID)->second.resending.end()) {
			clients.find(cID)->second.resending.erase(pID);
		}
	}*/
	//rebem resposta del ping i per tant encara esta conectat
	//fem reset del seu rellotge intern
	if (cmd == "ACK_PING") {
		if(clients.find(cID) != clients.end())
		clients.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == "DISCONNECTION") {
		NotifyOtherClients("DISCONNECTION", cID);
		clients[cID].connected = false;
	}
	else if (cmd == "NEWCONNECTION" && clients.size() != MAX_CLIENTS) {
		std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
		Position pos;
		srand(time(NULL));
		pos.x = std::rand() % 25;
		pos.y = std::rand() % 25;
		packet.clear();
		packet << "WELCOME" << packetID << clientID << pos.x << pos.y;
		if (clients.find(clientID) == clients.end()) {
			clients.insert(std::make_pair(clientID, Client{ clientID, pos, senderIP, senderPort, true }));
			clients[clientID].resending.insert(std::make_pair(packetID, packet));
			NotifyOtherClients("CONNECTION", clientID);
			SendToAllClients("POSITION");
			clientID++;
			packetID++;
		}
	}
	/*else {
		sf::Packet _packet;
		_packet << "ACK" << pID;
		if (clients.find(cID) != clients.end())
			clients.find(cID)->second.resending.insert(std::make_pair(pID, _packet));
	}*/
}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int8_t IDClient = 0;
	int8_t packetIDRecived = 0;
	std::string cmd;
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {
		packet >> cmd;
		if (cmd == "DISCONNECTION" || cmd == "ACK_PING") {
			packet >> IDClient;
			packetIDRecived = -1;
		}
		else
			packet >> packetIDRecived >> IDClient; //posar packetID quan tinguem id q envien els clients

		ManageReveivedData(cmd, IDClient, packetIDRecived, senderIP, senderPort, packet);

	}
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
		for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
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
	}
	//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	for (std::map<int8_t, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			NotifyOtherClients("DISCONNECTION", clientes->first);
			//clients.erase(clientes->first);
			clientes->second.connected = false;
		}
	}

	for (int8_t i = 0; i < clients.size(); i++) {
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
		//cada certa quantiat de temps enviar missatge ping
		if (clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			clockSend.restart();
		}
		ManagePing();

	} while (clients.size() >= 0);// && clients.size() <= MAX_CLIENTS);

	clients.clear();
	socket.unbind();
	system("exit");
	return 0;
}

//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include<GlobalValues.h>

//comandos
int8_t HELLO = 0;
int8_t ACK_HELLO = 1;
int8_t NEW_CONNECTION = 2;
int8_t ACK_NEW_CONNECTION = 3;
int8_t DISCONNECTION = 4;
int8_t ACK_DISCONNECTION = 5;
int8_t PING = 6;
int8_t ACK_PING = 7;

bool online = true;

sf::Socket::Status status;
int8_t clientID = 1;

std::map<int8_t, Client> clients;
sf::UdpSocket socket;

sf::Clock clockPing, clockSend;
bool once = false;
int8_t packetID = 1;
//variable para controlar cuando se han desconnectado todos hasta que hagamos los estados
int8_t clientsConnected = 0;

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
		}
	}
}



void SendToAllClients(int8_t cmd) {

	if (cmd == PING) {
		for (std::map<int8_t, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}
}

void NotifyOtherClients(int8_t cmd, int8_t cID) {
	if (cmd == NEW_CONNECTION) {
		if (clients.find(cID) != clients.end()) {
			for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
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
		for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
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


void ManageReveivedData(int8_t cmd, int8_t cID, int8_t pID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname) {


	if (cmd == ACK_PING) {
		if (clients.find(cID) != clients.end())
			clients.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == HELLO && clients.size() < MAX_CLIENTS) {
		//comprobar si el client ja esta connectat. Anteriorment comprovat amb el port pero es pot donar el cas d q es repeteixi el port
		bool alreadyConnected = false;
		for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->second.nickname == nickname) {
				alreadyConnected = true;
			}
		}

		Position pos;
		int8_t numOfOpponents = clients.size();
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
				for (std::map<int8_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
					packet << it->second.id << it->second.pos.x << it->second.pos.y;
				}
			}
			clients.insert(std::make_pair(clientID, Client{ clientID, nickname, pos, senderIP, senderPort, true }));
			NotifyOtherClients(NEW_CONNECTION, clientID);
			clients[clientID].timeElapsedLastPing.restart();
			clientsConnected++;
			//clients[clientID].resending.insert(std::make_pair(packetID, packet));
			clientID++;
			
		}
		status = socket.send(packet, senderIP, senderPort);
		if (status != sf::Socket::Done) {
			std::cout << "Error sending ACK_HELLO to client " << std::to_string(clientID - 1) << std::endl;
		}//else std::cout << "Sent ACK_HELLO to client " << std::to_string(clientID - 1) << std::endl;
		packet.clear();
	}
	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION) {
		if (clients.find(cID) != clients.end() && clients[cID].resending.find(pID) != clients[cID].resending.end()) {
			clients[cID].resending.erase(pID);
		}
	}

}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int8_t IDClient = 0;
	int8_t packetIDRecived = 0;
	int8_t cmd;
	std::string nickname = "";
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {
		packet >> cmd >> packetIDRecived;
		if (cmd == HELLO)
			packet >> nickname;
		else packet >> IDClient;
		ManageReveivedData(cmd, IDClient, packetIDRecived, senderIP, senderPort, nickname);
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

	//cada certa quantiat de temps enviar missatge ping
	if (clockPing.getElapsedTime().asMilliseconds() > _PING) {
		SendToAllClients(PING);
		clockPing.restart();
	}
	//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	for (std::map<int8_t, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			NotifyOtherClients(DISCONNECTION, clientes->first);
			//clients.erase(clientes->first);
			clientes->second.connected = false;
		}
	}

	for (int8_t i = 1; i <= clients.size(); i++) {
		if (clients.find(i) != clients.end() && !clients[i].connected) {
			std::cout << "Client " << std::to_string(clients[i].id) << " disconnected." << std::endl;
			clients.erase(clients[i].id);
		}
	}
}

int main()
{
	online = true;
	ControlServidor();
	clockSend.restart();
	clockPing.restart();
	//SI EL Q ES VOL ES Q NO SURTIN LES PESTANYES DE JUGAR FINS Q TOTS NO ESTIGUIN CONNECTATS, ALESHORES FER EL RECEIVE DEL WELCOME I EL SEND COM ESTAVA EN ELS ANTERIORS COMMITS
	do {
		ReceiveData();
		ManagePing();
		//cada certa quantiat de temps enviar missatge ping
		if (clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			clockSend.restart();
		}
		if (clients.size() <= 0 && clientsConnected == MAX_CLIENTS) online = false;
	} while (clients.size() <= MAX_CLIENTS && online);

	clients.clear();
	socket.unbind();
	system("exit");
	return 0;
}

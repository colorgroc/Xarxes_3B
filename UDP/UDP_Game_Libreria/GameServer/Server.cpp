//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include<GlobalValues.h>

bool online = true;

sf::Socket::Status status;
int32_t clientID = 1;

std::map<int32_t, Client> clients;
sf::UdpSocket socket;

sf::Clock clockPing, clockSend;
bool once = false;
int32_t packetID = 1;
//variable para controlar cuando se han desconnectado todos hasta que hagamos los estados
int32_t clientsConnected = 0;

sf::Clock clockPositions;
std::mutex myMutex;

Walls * myWalls;

void Resend() {
	
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	for (std::map<int32_t, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
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



void SendToAllClients(int cmd) {

	if (cmd == PING) {
		for (std::map<int32_t, Client>::iterator clientToSend = clients.begin(); clientToSend != clients.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			socket.send(packet, clientToSend->second.ip, clientToSend->second.port); //controlar errors
		}
	}
}

void NotifyOtherClients(int cmd, int32_t cID) {
	if (cmd == NEW_CONNECTION) {
		if (clients.find(cID) != clients.end()) {
			for (std::map<int32_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetID << cID << clients.find(cID)->second.pos;
					it->second.resending.insert(std::make_pair(packetID, packet));
				}
			} packetID++;
		}
	}
	else if (cmd == DISCONNECTION) {
		//if (clients.find(cID) != clients.end()) {
		for (std::map<int32_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
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
	else if (cmd == REFRESH_POSITIONS) { 
		if (clients.find(cID) != clients.end()) {
			for (std::map<int32_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetID << cID << clients.find(cID)->second.pos;
					socket.send(packet, it->second.ip, it->second.port);
				}
			} packetID++;
		}
	}
}


void ManageReveivedData(int cmd, int32_t cID, int32_t pID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname, int32_t idMovements, AccumMovements tryaccum) {


	if (cmd == ACK_PING) {
		if (clients.find(cID) != clients.end())
			clients.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == HELLO && clients.size() < MAX_CLIENTS) {
		//comprobar si el client ja esta connectat. Anteriorment comprovat amb el port pero es pot donar el cas d q es repeteixi el port
		bool alreadyConnected = false;
		for (std::map<int32_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
			if (it->second.nickname == nickname) {
				alreadyConnected = true;
			}
		}

		Position pos;
		int32_t numOfOpponents = clients.size();
		srand(time(NULL));
		pos.x = std::rand() % NUMBER_ROWS_COLUMNS;
		pos.y = std::rand() % NUMBER_ROWS_COLUMNS;
		pos = CellToPixel(pos.x ,pos.y);
		sf::Packet packet;
		packet.clear();
		if (!alreadyConnected) {
			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			packet << ACK_HELLO << pID << clientID << pos << numOfOpponents;
			if (numOfOpponents > 0) {
				//inserim al packet la ID i la pos de cada oponent
				for (std::map<int32_t, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
					packet << it->second.id << it->second.pos;
				}
			}
			clients.insert(std::make_pair(clientID, Client{ clientID, nickname, pos, senderIP, senderPort, true }));
			NotifyOtherClients(NEW_CONNECTION, clientID);
			clients[clientID].timeElapsedLastPing.restart();
			clientsConnected++;
			clientID++;
			
		}
		status = socket.send(packet, senderIP, senderPort);
		if (status != sf::Socket::Done) {
			std::cout << "Error sending ACK_HELLO to client " << std::to_string(clientID - 1) << std::endl;
		}
		packet.clear();
	}
	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION || cmd == ACK_REFRESH_POSITIONS) {
		if (clients.find(cID) != clients.end() && clients[cID].resending.find(pID) != clients[cID].resending.end()) {
			clients[cID].resending.erase(pID);
		}
	}
	else if (cmd == TRY_POSITION) {

		//posarlo a dintre duna llista per m�s tard fer les validacions, una vegada fetes les validacions es borra la llista de moviments acumulats

		if (clients.find(cID)->second.MapAccumMovements.empty()) { //es el primer paquet del client, per tant no acumulem, si no que inicialitzem el mapa
			clients.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, tryaccum));
		}
		else { //hem d'acumular amb l'anterior paquet accum, posar el id move del ultim, sumar deltes i posicio del ultim
			AccumMovements temp;
			temp.delta.x =  clients.find(cID)->second.MapAccumMovements.rbegin()->second.delta.x += tryaccum.delta.x;
			temp.delta.y  = clients.find(cID)->second.MapAccumMovements.rbegin()->second.delta.y += tryaccum.delta.y;
			temp.absolute = tryaccum.absolute;
			clients.find(cID)->second.MapAccumMovements.erase(clients.find(cID)->second.MapAccumMovements.rbegin()->first); //borrem anterior
			clients.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, temp)); //insertem amb els nous valors actualitzats
			
		}

	}

}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	sf::IpAddress senderIP;
	unsigned short senderPort;
	int32_t IDClient = 0;
	int32_t packetIDRecived = 0;
	int cmd;
	std::string nickname = "";
	AccumMovements tryaccum = AccumMovements{ Position{0, 0},  Position{ 0, 0 } };
	int32_t idMovements = 0;
	status = socket.receive(packet, senderIP, senderPort);

	if (status == sf::Socket::Done) {
		packet >> cmd >> packetIDRecived;
		if (cmd == HELLO) {
			packet >> nickname;
		}
		else {
			packet >> IDClient;

			if (cmd == TRY_POSITION) {
				packet >> idMovements;
				packet >> tryaccum;
			}
		}
		ManageReveivedData(cmd, IDClient, packetIDRecived, senderIP, senderPort, nickname, idMovements, tryaccum);
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
	//quan enviem el missatge ping tamb� comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	for (std::map<int32_t, Client>::iterator clientes = clients.begin(); clientes != clients.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			NotifyOtherClients(DISCONNECTION, clientes->first);
			clientes->second.connected = false;
		}
	}

	for (int32_t i = 1; i <= clients.size(); i++) {
		if (clients.find(i) != clients.end() && !clients[i].connected) {
			std::cout << "Client " << std::to_string(clients[i].id) << " disconnected." << std::endl;
			clients.erase(clients[i].id);
		}
	}
}

void PositionValidations() {
	//recorre tota la llista de clients
	//anar validant i descartant
	//enviar amb OkPosition
	//reenviar a tots els altres jugadors
	//REFRESH_POSITIONS 
	

	for (std::map<int32_t, Client>::iterator client = clients.begin(); client != clients.end(); ++client) {

		std::vector<int32_t> posToDelete;

		for (std::map<int32_t, AccumMovements>::iterator pos = client->second.MapAccumMovements.begin(); pos != client->second.MapAccumMovements.end(); ++pos) {

			if (myWalls->CheckCollision(pos->second)) {
				client->second.pos = pos->second.absolute; //actualitzo posicio ja que es correcta

																  //enviem i notifiquem
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << client->second.pos;
				status = socket.send(packet, client->second.ip, client->second.port);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending OK_POSITION to client " << std::to_string(client->second.id) << std::endl;
				}
				if (clients.size() > 1) {
					NotifyOtherClients(REFRESH_POSITIONS, client->second.id); 
				}
			
				packet.clear();

			}
			else {
				//posem posicion -1 -1 per que el client pugui eliminarlo i que no es mogui
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << Position{-1,-1};
				status = socket.send(packet, client->second.ip, client->second.port);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending OK_POSITION to client " << std::to_string(client->second.id) << std::endl;
				}
				packet.clear();
			}
			//guardem per borrar
			posToDelete.push_back(pos->first);
		}
		//borrem tots els accum analitzats de la llista del client
		for (std::vector<int32_t>::iterator toDelete = posToDelete.begin(); toDelete != posToDelete.end(); ++toDelete) {
			client->second.MapAccumMovements.erase(*toDelete);
		}
	
	}
}

int main()
{
	online = true;
	ControlServidor();
	clockSend.restart();
	clockPing.restart();
	
	myWalls = new Walls(); //ara el server ja sap on hi ha parets

	do {
		ReceiveData();
		ManagePing();

		//validem moviments jugadors
		if(clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS){
			PositionValidations();
			clockPositions.restart();
		}

		//cada certa quantiat de temps enviar missatges
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

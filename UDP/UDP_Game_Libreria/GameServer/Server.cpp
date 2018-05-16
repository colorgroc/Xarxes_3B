//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include<GlobalValues.h>

bool online = true;

sf::Socket::Status status;
int32_t clientID = 1;

std::map<int32_t, Client> clientsOnLobby;
sf::UdpSocket socket;

sf::Clock clockPing, clockSend;
int32_t packetID = 1;
//variable para controlar cuando se han desconnectado todos hasta que hagamos los estados
//int32_t clientsConnected = 0;
std::vector<int32_t> idsDesaprovechadas;
std::set<int32_t> pillados;
sf::Clock clockPositions;
std::mutex myMutex;
bool gameStarted = false;
bool once = false;
Walls * myWalls;
int32_t receivedWinner = 0;
std::map<int8_t, Partida> partidas;

void Resend() {
	
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	for (std::map<int32_t, Player>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end(); ++clientes) {
		for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			status = socket.send(msg->second, clientes->second.ip, clientes->second.port);
			if (status == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			else if (status == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clientsOnLobby.erase(clientes);
			}
		}
	}
}



void SendToAllClients(int cmd) {

	if (cmd == PING) {
		for (std::map<int32_t, Player>::iterator clientToSend = clientsOnLobby.begin(); clientToSend != clientsOnLobby.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			status = socket.send(packet, clientToSend->second.ip, clientToSend->second.port); 
			if (status != sf::Socket::Done) {
				std::cout << "Error sending PING to client " << std::to_string(clientToSend->first) << std::endl;
			}
		}
	}
	else if (cmd == GAMESTARTED) { //no es critic
		for (std::map<int32_t, Player>::iterator clientToSend = clientsOnLobby.begin(); clientToSend != clientsOnLobby.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			status = socket.send(packet, clientToSend->second.ip, clientToSend->second.port); 
			if (status != sf::Socket::Done) {
				std::cout << "Error sending GAMESTARTED to client " << std::to_string(clientToSend->first) << std::endl;
			}
		}
	}
	
}

void NotifyOtherClients(int cmd, int32_t cID) {
	if (cmd == NEW_CONNECTION) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetID << cID << clientsOnLobby.find(cID)->second.pos;
					it->second.resending.insert(std::make_pair(packetID, packet));
				}
			} packetID++;
		}
	}
	else if (cmd == DISCONNECTION) {
		//if (clients.find(cID) != clients.end()) {
		for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
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
		if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetID << cID << clientsOnLobby.find(cID)->second.pos;
					status = socket.send(packet, it->second.ip, it->second.port);
					if (status != sf::Socket::Done) {
						std::cout << "Error sending REFRESH_POSITIONS to client " << std::to_string(cID) << std::endl;
					}
				}
			} packetID++;
		}
	}
	else if (cmd == QUI_LA_PILLA) {
		//if (clients.find(cID) != clients.end()) {
		for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd;
			packet << packetID << cID;
			it->second.resending.insert(std::make_pair(packetID, packet));
		}packetID++;
		//}
	}
	else if (cmd == WINNER) {
		//if (clients.find(cID) != clients.end()) {
		for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd;
			packet << packetID << cID;
			it->second.resending.insert(std::make_pair(packetID, packet));
		}packetID++;
		//}
	}
}
void PilladorRandom() {
	std::uniform_int_distribution<unsigned short> num(1, clientsOnLobby.size());
	std::random_device rd;
	std::mt19937 gen(rd());
	int32_t pillador = num(gen);

	for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
		if (it->second.id == pillador) {
			it->second.laPara = true;
			std::cout << "La pilla el client amb nickname: " << it->second.nickname << " i amb ID: " << std::to_string(it->first) << std::endl;
			NotifyOtherClients(QUI_LA_PILLA, it->second.id);
		}
	}
	//std::cout << pillador << std::endl;
	/*for (int i = 1; i <= clients.size(); i++) {
		if (clients.find(i) != clients.end() && clients[i].id == pillador) {
			clients[i].laPara = true;
			std::cout << "La pilla el client amb nickname: " << clients[i].nickname << " i amb ID: " << clients[i].id << std::endl;
			NotifyOtherClients(QUI_LA_PILLA, clients[i].id);
		}
	}*/
}
bool CheckCollisionWithClientsPos(Position pos) { //amb pixels
	bool correctPosition = true;

	for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {

		if (pos.x == it->second.pos.x && pos.y == it->second.pos.y)
			correctPosition = false;
	}
	return correctPosition;
}

void ManageReveivedData(int cmd, int32_t cID, int32_t pID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname, int32_t idMovements, AccumMovements tryaccum, int32_t idOpponentCollision, int8_t idPartida) {


	if (cmd == ACK_PING) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end())
			clientsOnLobby.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == HELLO && clientsOnLobby.size() < MAX_CLIENTS) {
		//comprobar si el client ja esta connectat. Anteriorment comprovat amb el port pero es pot donar el cas d q es repeteixi el port
		bool alreadyConnected = false;
		for (std::map<int32_t, Player>::iterator it = partidas.find(idPartida)->second.jugadors.begin(); it != partidas.find(idPartida)->second.jugadors.end(); ++it) {//aqui serien clients lobby --> canviarho
			if (it->second.nickname == nickname || it->second.port == senderPort) {
				alreadyConnected = true;
				sf::Packet p;
				p << ID_ALREADY_TAKEN;
				status = socket.send(p, senderIP, senderPort);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending ID_ALREADY_TAKEN to unknown client. " << std::endl;
				}
			}
		}

		Position pos;
		int32_t numOfOpponents = clientsOnLobby.size();
		srand(time(NULL));
		
		//random range c++11 stuff
		do {
			std::random_device rdX, rdY;
			std::mt19937 genX(rdX()), genY(rdY());
			std::uniform_int_distribution<int16_t> num(1, NUMBER_ROWS_COLUMNS - 2);
			pos.x = num(genX);
			pos.y = num(genY);
			//pos.x = GetRandomFloat(1, NUMBER_ROWS_COLUMNS - 1);
			//pos.y = GetRandomFloat(1, NUMBER_ROWS_COLUMNS - 1);
			//std::cout << "yeah!" << std::endl;
		} while (!myWalls->CheckCollision(pos) && !CheckCollisionWithClientsPos(pos));
		
		pos = CellToPixel(pos.x ,pos.y);
		sf::Packet packet;
		packet.clear();

		if (!alreadyConnected) {
			
			if (idsDesaprovechadas.empty()) {
				std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
				packet << ACK_HELLO << pID << clientID << pos << numOfOpponents;
				if (numOfOpponents > 0) {
					//inserim al packet la ID i la pos de cada oponent
					for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
						packet << it->second.id << it->second.pos;
					}
				}
				clientsOnLobby.insert(std::make_pair(clientID, Player{ clientID, nickname, pos, senderIP, senderPort, true, false, false }));
				NotifyOtherClients(NEW_CONNECTION, clientID);
				clientsOnLobby[clientID].timeElapsedLastPing.restart();
				//clientsConnected++;
				clientID++;

				status = socket.send(packet, senderIP, senderPort);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending ACK_HELLO to client " << std::to_string(clientID - 1) << std::endl;
				}
				packet.clear();
			}
			else {
				std::cout << "Connection with client " << std::to_string(idsDesaprovechadas[0]) << " from PORT " << senderPort << std::endl;
				packet << ACK_HELLO << pID << idsDesaprovechadas[0] << pos << numOfOpponents;
				if (numOfOpponents > 0) {
					//inserim al packet la ID i la pos de cada oponent
					for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
						packet << it->second.id << it->second.pos;
					}
				}
				clientsOnLobby.insert(std::make_pair(idsDesaprovechadas[0], Player{ idsDesaprovechadas[0], nickname, pos, senderIP, senderPort, true, false, false }));
				NotifyOtherClients(NEW_CONNECTION, idsDesaprovechadas[0]);
				clientsOnLobby[idsDesaprovechadas[0]].timeElapsedLastPing.restart();
				//clientsConnected++;

				status = socket.send(packet, senderIP, senderPort);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending ACK_HELLO to client " << std::to_string(idsDesaprovechadas[0]) << std::endl;
				}
				packet.clear();
				idsDesaprovechadas.erase(idsDesaprovechadas.begin());
			}
			
		}
		
	}

	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION || cmd == ACK_QUI_LA_PILLA || cmd == ACK_WINNER) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end() && clientsOnLobby[cID].resending.find(pID) != clientsOnLobby[cID].resending.end()) {
			clientsOnLobby[cID].resending.erase(pID);
			if (cmd == ACK_WINNER) {
				receivedWinner++;
			}
		}
	}
	else if (cmd == TRY_POSITION) {

		//posarlo a dintre duna llista per més tard fer les validacions, una vegada fetes les validacions es borra la llista de moviments acumulats
		if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			if (clientsOnLobby.find(cID)->second.MapAccumMovements.empty()) { //es el primer paquet del client, per tant no acumulem, si no que inicialitzem el mapa
				clientsOnLobby.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, tryaccum));
			}
			else { //hem d'acumular amb l'anterior paquet accum, posar el id move del ultim, sumar deltes i posicio del ultim
				AccumMovements temp;
				temp.delta.x = clientsOnLobby.find(cID)->second.MapAccumMovements.rbegin()->second.delta.x += tryaccum.delta.x;
				temp.delta.y = clientsOnLobby.find(cID)->second.MapAccumMovements.rbegin()->second.delta.y += tryaccum.delta.y;
				temp.absolute = tryaccum.absolute;
				clientsOnLobby.find(cID)->second.MapAccumMovements.erase(clientsOnLobby.find(cID)->second.MapAccumMovements.rbegin()->first); //borrem anterior
				clientsOnLobby.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, temp)); //insertem amb els nous valors actualitzats
			}
		}

	}
	else if(TRY_COLLISION_OPPONENT) {
		if (clientsOnLobby.find(idOpponentCollision) != clientsOnLobby.end() && clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			if (clientsOnLobby.find(idOpponentCollision)->second.pos.x <= clientsOnLobby.find(cID)->second.pos.x + 15 && clientsOnLobby.find(idOpponentCollision)->second.pos.x >= clientsOnLobby.find(cID)->second.pos.x - 15 && clientsOnLobby.find(idOpponentCollision)->second.pos.y <= clientsOnLobby.find(cID)->second.pos.y + 15 && clientsOnLobby.find(idOpponentCollision)->second.pos.y >= clientsOnLobby.find(cID)->second.pos.y - 15) {
				//std::cout << "Collision With Opponent " << idOpponentCollision << "  " << cID << std::endl;
				if (!clientsOnLobby[idOpponentCollision].laPara && clientsOnLobby[cID].laPara) {
					clientsOnLobby[idOpponentCollision].laPara = true;
					NotifyOtherClients(QUI_LA_PILLA, idOpponentCollision);
				}
				else if (clientsOnLobby[idOpponentCollision].laPara && !clientsOnLobby[cID].laPara) {
					clientsOnLobby[cID].laPara = true;
					NotifyOtherClients(QUI_LA_PILLA, cID);
				}
			}
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
	//int32_t sizeMovements = 0;
	status = socket.receive(packet, senderIP, senderPort);
	int32_t idOpponentCollision = 0;

	if (status == sf::Socket::Done) {
		float rndPacketLoss = GetRandomFloat();
		if(rndPacketLoss < PERCENT_PACKETLOSS){
			std::cout << "Paquet perdut" << std::endl;
		}
		else {
			packet >> cmd >> packetIDRecived;
			if (cmd == HELLO) {
				packet >> nickname;
			}
			else {
				packet >> IDClient;

				if (cmd == TRY_POSITION) {
					packet >> idMovements >> tryaccum;
					//packet >> tryaccum;
				}

				if(cmd == TRY_COLLISION_OPPONENT){
					packet >> idOpponentCollision;
				}

			}
			ManageReveivedData(cmd, IDClient, packetIDRecived, senderIP, senderPort, nickname, idMovements, tryaccum, idOpponentCollision);
		}
		
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
	for (std::map<int32_t, Player>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			NotifyOtherClients(DISCONNECTION, clientes->first);
			clientes->second.connected = false;
			if (!gameStarted) {
				//clientsConnected--;
				idsDesaprovechadas.push_back(clientes->first);
			}
			else {
				if (pillados.size() == 1 && clientes->second.laPara) {
					pillados.erase(clientes->first);
					PilladorRandom();
				}
			}
		}
	}

	for (std::map<int32_t, Player>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end();) {
		if (!clientes->second.connected) {
			std::cout << "Client " << std::to_string(clientes->first) << " disconnected." << std::endl;
			clientes = clientsOnLobby.erase(clientes);
		}
		else ++clientes;
	}

}

void PositionValidations() {
	//recorre tota la llista de clients
	//anar validant i descartant
	//enviar amb OkPosition
	//reenviar a tots els altres jugadors
	//REFRESH_POSITIONS 
	

	for (std::map<int32_t, Player>::iterator client = clientsOnLobby.begin(); client != clientsOnLobby.end(); ++client) {

		std::vector<int32_t> posToDelete;
		Position lastPos = client->second.pos;
		for (std::map<int32_t, AccumMovements>::iterator pos = client->second.MapAccumMovements.begin(); pos != client->second.MapAccumMovements.end(); ++pos) {

			if (myWalls->CheckCollision(pos->second)) {
				client->second.pos = pos->second.absolute; //actualitzo posicio ja que es correcta
				lastPos = client->second.pos;
																  //enviem i notifiquem
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << client->second.pos;
				status = socket.send(packet, client->second.ip, client->second.port);
				if (status != sf::Socket::Done) {
					std::cout << "Error sending OK_POSITION to client " << std::to_string(client->second.id) << std::endl;
				}
				if (clientsOnLobby.size() > 1) {
					NotifyOtherClients(REFRESH_POSITIONS, client->second.id); 
				}
			
				packet.clear();

			}
			else {
				//posem posicion -1 -1 per que el client pugui eliminarlo i que no es mogui
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << Position{ -1,-1 } << Position{lastPos};
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

void ComprovacioPillats() {
	for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
		if (it->second.laPara)
			pillados.insert(it->first);
	}

}
void Winner() {
	if (gameStarted && pillados.size() >= (clientsOnLobby.size()-1)) {
		for (std::map<int32_t, Player>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
			if (!it->second.laPara) {
				it->second.winner = true;
				NotifyOtherClients(WINNER, it->first);
			}
		}
		//online = false;
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
		Winner();
		//validem moviments jugadors
		if(clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS){
			PositionValidations();
			clockPositions.restart();
		}

		//cada certa quantiat de temps enviar missatges
		if (clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			ComprovacioPillats();
			clockSend.restart();
		}
		if (clientsOnLobby.size() >= MAX_CLIENTS && !gameStarted){// && clientsConnected == MAX_CLIENTS) {
			//game starts!
			gameStarted  = true;
			SendToAllClients(GAMESTARTED);
		}
		if (gameStarted && !once) {
			PilladorRandom();
			once = true;
		}
		if ((clientsOnLobby.size() <= 0 && gameStarted) || (receivedWinner >= clientsOnLobby.size() && gameStarted)) online = false;

	} while (online);

	clientsOnLobby.clear();
	socket.unbind();
	system("exit");
	return 0;
}

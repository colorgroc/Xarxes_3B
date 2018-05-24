//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include<GlobalValues.h>


std::map<int32_t, ClientLobby> clientsOnLobby;
int32_t packetIDLobby = 1;
sf::Clock clockPingLobby, clockSendLobby;
int32_t idPartida = 1;

bool online = true;

std::mutex myMutex;
std::map<int32_t, Partida*> partidas;
sf::UdpSocket socketServer;
sf::Socket::Status statusServer;
int32_t clientID = 1;
Walls * myWalls;
//bool online = true;

void Resend(int32_t gID) {

	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	for (std::map<int32_t, Player>::iterator clientes = partidas[gID]->jugadors.begin(); clientes != partidas[gID]->jugadors.end(); ++clientes) {
		for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			statusServer = socketServer.send(msg->second, clientes->second.ip, clientes->second.port);
			if (statusServer == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			else if (statusServer == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				partidas[gID]->jugadors.erase(clientes);
			}
		}
	}
}
void ResendLobby() {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end(); ++clientes) {
		for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			statusServer = socketServer.send(msg->second, clientes->second.ip, clientes->second.port);
			if (statusServer == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			else if (statusServer == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clientsOnLobby.erase(clientes);
			}
		}
	}
}

void SendToAllClients(int cmd, int32_t gID) {

	if (cmd == PING_LOBBY) {
		for (std::map<int32_t, ClientLobby>::iterator clientToSend = clientsOnLobby.begin(); clientToSend != clientsOnLobby.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port); 
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending PING to client " << std::to_string(clientToSend->first) << std::endl;
			}
		}
	}
	if (cmd == PING) {

		for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID]->jugadors.begin(); clientToSend != partidas[gID]->jugadors.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending PING to client " << std::to_string(clientToSend->first) << std::endl;
			}
		}
	}
	else if (cmd == GAMESTARTED) { //no es critic

		for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID]->jugadors.begin(); clientToSend != partidas[gID]->jugadors.end(); ++clientToSend)
		{
			sf::Packet packet;
			packet << cmd; //no hace falta poner packetID 
			statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending GAMESTARTED to client " << std::to_string(clientToSend->first) << std::endl;
			}
		}
	}
	else if (cmd == GAME_DELETED) {
		for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd << packetIDLobby << gID;
			it->second.resending.insert(std::make_pair(packetIDLobby, packet));

		}packetIDLobby++;
	}
}

void NotifyOtherClients(int cmd, int32_t cID, std::string msg, int32_t gID, int32_t maxPlayers) {
	//if (cmd == NEW_CONNECTION_LOBBY) { //si sou amics XD
	//	if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
	//		for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
	//		{
	//			sf::Packet packet;
	//			if (it->first != cID) {
	//				packet << cmd << packetIDLobby << cID;// << clientsOnLobby.find(cID)->second.nickname;
	//				it->second.resending.insert(std::make_pair(packetIDLobby, packet));
	//			}
	//		} packetIDLobby++;
	//	}
	//}
	//else if (cmd == DISCONNECTION_LOBBY) { //si sou amics XD

	//	for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
	//	{
	//		sf::Packet packet;
	//		packet << cmd;
	//		if (it->first != cID) {
	//			packet << packetIDLobby << cID;
	//			it->second.resending.insert(std::make_pair(packetIDLobby, packet));
	//		}
	//	}packetIDLobby++;
	//}
	if (cmd == NEW_GAME_CREATED) { 
		if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << packetIDLobby << gID << msg << maxPlayers;// << clientsOnLobby.find(cID)->second.nickname;
					it->second.resending.insert(std::make_pair(packetIDLobby, packet));
				}
			} packetIDLobby++;
		}
	}
	
	if (cmd == NEW_CONNECTION) {
		if (partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << partidas[gID]->packetID << cID << partidas[gID]->jugadors.find(cID)->second.nickname << partidas[gID]->jugadors.find(cID)->second.pos;
					it->second.resending.insert(std::make_pair(partidas[gID]->packetID, packet));
				}
			} partidas[gID]->packetID++;
		}
	}
	else if (cmd == DISCONNECTION) {
		for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd;
			if (it->first != cID) {
				packet << partidas[gID]->packetID << cID;
				it->second.resending.insert(std::make_pair(partidas[gID]->packetID, packet));
			}
		}partidas[gID]->packetID++;

	}
	else if (cmd == GLOBAL_CHAT) { //no es critic
		for (std::map<int32_t, ClientLobby>::iterator clientToSend = clientsOnLobby.begin(); clientToSend != clientsOnLobby.end(); ++clientToSend)
		{
			if (clientToSend->first != cID) {
				sf::Packet packet;
				packet << cmd << clientToSend->second.nickname << msg; //no hace falta poner packetID 
				statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending GLOBAL_CHAT to client " << std::to_string(clientToSend->first) << std::endl;
				}
			}
		}
	}
	else if (cmd == GAME_CHAT) { //no es critic
		for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID]->jugadors.begin(); clientToSend != partidas[gID]->jugadors.end(); ++clientToSend)
		{
			if (clientToSend->first != cID) {
				sf::Packet packet;
				packet << cmd << clientToSend->second.nickname << msg; //no hace falta poner packetID 
				statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending GAME_CHAT to client " << std::to_string(clientToSend->first) << std::endl;
				}
			}
		}
	}
	else if (cmd == REFRESH_POSITIONS) {
		if (partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << partidas[gID]->packetID << cID << partidas[gID]->jugadors.find(cID)->second.pos;
					statusServer = socketServer.send(packet, it->second.ip, it->second.port);
					if (statusServer != sf::Socket::Done) {
						std::cout << "Error sending REFRESH_POSITIONS to client " << std::to_string(cID) << std::endl;
					}
				}
			} partidas[gID]->packetID++;
		}
	}
	else if (cmd == QUI_LA_PILLA) {
		for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd;
			packet << partidas[gID]->packetID << cID;
			it->second.resending.insert(std::make_pair(partidas[gID]->packetID, packet));
		}partidas[gID]->packetID++;
	}
	else if (cmd == WINNER) {

		for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it)
		{
			sf::Packet packet;
			packet << cmd;
			packet << partidas[gID]->packetID << cID;
			it->second.resending.insert(std::make_pair(partidas[gID]->packetID, packet));
		}partidas[gID]->packetID++;
	}
}

void PilladorRandom(int32_t gID) {
	std::uniform_int_distribution<unsigned short> num(1, partidas[gID]->jugadors.size());
	std::random_device rd;
	std::mt19937 gen(rd());
	int32_t pillador = num(gen);

	for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it) {
		if (it->second.id == pillador) {
			it->second.laPara = true;
			std::cout << "La pilla el client amb nickname: " << it->second.nickname << " i amb ID: " << std::to_string(it->first) << std::endl;
			NotifyOtherClients(QUI_LA_PILLA, it->second.id, "", gID, 0);
		}
	}
}

void PositionValidations(int32_t gID) {
	//recorre tota la llista de clients
	//anar validant i descartant
	//enviar amb OkPosition
	//reenviar a tots els altres jugadors
	//REFRESH_POSITIONS 

	for (std::map<int32_t, Player>::iterator client = partidas[gID]->jugadors.begin(); client != partidas[gID]->jugadors.end(); ++client) {

		std::vector<int32_t> posToDelete;
		Position lastPos = client->second.pos;
		for (std::map<int32_t, AccumMovements>::iterator pos = client->second.MapAccumMovements.begin(); pos != client->second.MapAccumMovements.end(); ++pos) {

			if (myWalls->CheckCollision(pos->second)) {
				client->second.pos = pos->second.absolute; //actualitzo posicio ja que es correcta
				lastPos = client->second.pos;
				//enviem i notifiquem
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << client->second.pos;
				statusServer = socketServer.send(packet, client->second.ip, client->second.port);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending OK_POSITION to client " << std::to_string(client->second.id) << std::endl;
				}
				if (partidas[gID]->jugadors.size() > 1) {
					NotifyOtherClients(REFRESH_POSITIONS, client->second.id, "", gID, 0);
				}

				packet.clear();
			}
			else {
				//posem posicion -1 -1 per que el client pugui eliminarlo i que no es mogui
				sf::Packet packet;
				packet << OK_POSITION << pos->first << pos->first << Position{ -1,-1 } << Position{ lastPos };
				statusServer = socketServer.send(packet, client->second.ip, client->second.port);
				if (statusServer != sf::Socket::Done) {
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

void Winner(int32_t gID) {
	if (partidas[gID]->gameStarted && partidas[gID]->pillados.size() >= (partidas[gID]->jugadors.size() - 1)) {
		for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it) {
			if (!it->second.laPara) {
				it->second.winner = true;
				NotifyOtherClients(WINNER, it->first, "", gID, 0);
			}
		}
	}
}

void ManageReveivedData(int cmd, int32_t cID, int32_t pID, int32_t gID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname, std::string mail, std::string password, std::string msg, std::string namePartida, std::string passwordPartida, int32_t maxPlayers, int32_t idMovements, AccumMovements tryaccum, int32_t idOpponentCollision) {

	if (cmd == ACK_PING_LOBBY) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end())
			clientsOnLobby.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == ACK_PING) {
		if (partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end())
			partidas[gID]->jugadors.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == LOGIN) { 
		//comprobar si el client ja esta connectat. 
		bool alreadyConnected = false;
		bool notExists = false;
		//fer un if(find(nickname) de la base de dades) --> si no existeix el nickname o la password no concorda, noExists = true;
		//std::cout << "Login" << std::endl;
		//for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
		//	if ((it->second.nickname == nickname || it->second.port == senderPort) && it->second.connected) {
		//		alreadyConnected = true;
		//		sf::Packet p;
		//		p << ID_ALREADY_CONNECTED << pID; // posar lu d q retorni -1,-2 o -3
		//		statusServer = socketServer.send(p, senderIP, senderPort);
		//		if (statusServer != sf::Socket::Done) {
		//			std::cout << "Error sending ID_ALREADY_CONNECTED to unknown client. " << std::endl;
		//		}
		//	}
		//}

		sf::Packet packet;
		packet.clear();

		if (!alreadyConnected && !notExists) {
			//trobar id a la base d dades
			//int32_t baseIDClient = 0;
			//int32_t numPartidas = partidas.size();	
			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			packet << ACK_LOGIN << pID << clientID << partidas.size();
			if (partidas.size() > 0) {
				for (int8_t i = 0; i < partidas.size(); i++) {
					packet << partidas[i]->id << partidas[i]->name << partidas[i]->jugadors.size() << partidas[i]->maxPlayers;
				}
			}
			clientsOnLobby.insert(std::make_pair(clientID, ClientLobby{ clientID, nickname, senderIP, senderPort, 1, 0, 0, true }));
			//ACTUALITZAR BASE DE DADES --> ELS NUMS AL INSERTAR NO HA DANAR AIXI 

			//NotifyOtherClients(NEW_CONNECTION, clientID);
			clientsOnLobby[clientID].timeElapsedLastPing.restart();
			clientID++;

			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_LOGIN to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}

	}
	else if (cmd == SIGNUP) { 
																	 
		bool alreadyExists = false;
		//std::cout << "SingUp" << std::endl;
		//fer un if(find(nickname) de la base de dades) --> si ja existeix el nickname i el mail --> alreadyExists = true; 
		//enviar al client ID_ALREADY_EXISTS (MAIL O NICKNAME)
		//aixo es temporal
		//for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it) {
		//	if ((it->second.nickname == nickname || it->second.port == senderPort) && it->second.connected) {
		//		alreadyExists = true;
		//		sf::Packet p;
		//		p << ID_ALREADY_CONNECTED; // posar lu d q retorni -1,-2 o -3
		//		statusServer = socketServer.send(p, senderIP, senderPort);
		//		if (statusServer != sf::Socket::Done) {
		//			std::cout << "Error sending ID_ALREADY_CONNECTED to unknown client. " << std::endl;
		//		}
		//	}
		//}

		sf::Packet packet;
		packet.clear();

		if (!alreadyExists) {

			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			//pillar la  base ID del client a la base d dades;
			packet << ACK_SIGNUP << pID <<  clientID << partidas.size();
			if (partidas.size() > 0) {
				for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
					packet << it->first;
					packet << it->second->name;
					packet << it->second->jugadors.size();
					packet << it->second->maxPlayers;
				}
				/*for (int8_t i = 0; i < partidas.size(); i++) {
					packet << partidas[i];
					packet << partidas[i]->name;
					if(partidas[i]->jugadors.size() > 0)
					packet << partidas[i]->jugadors.size();
					else packet << 0;
					packet << partidas[i]->maxPlayers;
				}*/
			}
			clientsOnLobby.insert(std::make_pair(clientID, ClientLobby{ clientID, nickname, senderIP, senderPort, 1, 0, 0, true }));
			//AFEGIR A LA BASE DE DADES

			//NotifyOtherClients(NEW_CONNECTION, clientID);
			clientsOnLobby[clientID].timeElapsedLastPing.restart();
			clientID++;

			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_SIGNUP to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}
	}
	else if (cmd == GLOBAL_CHAT || cmd == GAME_CHAT) {
		NotifyOtherClients(cmd, cID, msg, gID, 0);
	}
	else if (cmd == NEW_GAME) {
	
		Position pos;
		int32_t numOfOpponents = 0;
		srand(time(NULL));
		std::cout << "CREATE" << std::endl;
		//random range c++11 stuff
		do {
			std::random_device rdX, rdY;
			std::mt19937 genX(rdX()), genY(rdY());
			std::uniform_int_distribution<int16_t> num(1, NUMBER_ROWS_COLUMNS - 2);
			pos.x = num(genX);
			pos.y = num(genY);
		} while (!myWalls->CheckCollision(pos));

		pos = CellToPixel(pos.x, pos.y);
		sf::Packet packet;
		packet << WELCOME << pID << gID << pos << numOfOpponents;
		partidas.insert(std::make_pair(idPartida, new Partida(idPartida, cID, namePartida, passwordPartida, maxPlayers)));
		statusServer = socketServer.send(packet, senderIP, senderPort);
		if (statusServer != sf::Socket::Done) {
			std::cout << "Error sending WELCOME to client " << std::to_string(clientID - 1) << std::endl;
		}
		packet.clear();
		NotifyOtherClients(NEW_GAME_CREATED, cID, "", idPartida, maxPlayers);
		idPartida++;
		packet.clear();

	}

	else if (cmd == JOIN_GAME && partidas[gID]->jugadors.size() < partidas[gID]->maxPlayers) {
																 
		bool alreadyConnected = false;
		bool correctPassword = true; //per ara deixarla a true
		//COMPROVAR SI LA PASSWORD ES CORRECTE SI NO HO ÉS, ENVIAR AL JUGADOR PASSWORD_INCORRECT --> BASE DE DADES <-- si es correcte, correctPassword = true
		std::cout << "JOIN" << std::endl;
		//si aquest usuari ja esta jugant
		/*for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it) {
			if (it->second.nickname == nickname || it->second.port == senderPort) {
				alreadyConnected = true;
				sf::Packet p;
				p << ID_ALREADY_PLAYING << pID;
				statusServer = socketServer.send(p, senderIP, senderPort);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending ID_ALREADY_PLAYING to unknown client. " << std::endl;
				}
			}
		}*/

		Position pos;
		int32_t numOfOpponents = partidas[gID]->jugadors.size();
		srand(time(NULL));

		//random range c++11 stuff
		do {
			std::random_device rdX, rdY;
			std::mt19937 genX(rdX()), genY(rdY());
			std::uniform_int_distribution<int16_t> num(1, NUMBER_ROWS_COLUMNS - 2);
			pos.x = num(genX);
			pos.y = num(genY);

		} while (!myWalls->CheckCollision(pos) && !partidas[gID]->CheckCollisionWithClientsPos(pos));

		pos = CellToPixel(pos.x, pos.y);
		sf::Packet packet;
		packet.clear();

		if (!alreadyConnected && correctPassword) {
			
			packet << WELCOME << pID << gID << pos << numOfOpponents;
			if (numOfOpponents > 0) {
				//inserim al packet la ID i la pos de cada oponent
				for (std::map<int32_t, Player>::iterator it = partidas[gID]->jugadors.begin(); it != partidas[gID]->jugadors.end(); ++it) {
					packet << it->second.id <<  it->second.nickname << it->second.pos;
				}
			}
			partidas[gID]->jugadors.insert(std::make_pair(clientID, Player{ cID, nickname, pos, senderIP, senderPort, true, false, false }));
			//ACTUALITZAR BASE DE DADES del jugador --> partides jugades

			NotifyOtherClients(NEW_CONNECTION, cID, "", gID, 0);
			partidas[gID]->jugadors[clientID].timeElapsedLastPing.restart();
			
			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending WELCOME to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}
	}

	else if (cmd == ACK_NEW_CONNECTION_LOBBY || cmd == ACK_DISCONNECTION_LOBBY) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end() && clientsOnLobby[cID].resending.find(pID) != clientsOnLobby[cID].resending.end()) {
			clientsOnLobby[cID].resending.erase(pID);
		}
	}

	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION || cmd == ACK_QUI_LA_PILLA || cmd == ACK_WINNER) {
		if (partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end() && partidas[gID]->jugadors[cID].resending.find(pID) != partidas[gID]->jugadors[cID].resending.end()) {
			partidas[gID]->jugadors[cID].resending.erase(pID);
			if (cmd == ACK_WINNER) {
				partidas[gID]->receivedWinner++;
			}
		}
	}	
	
	else if (cmd == TRY_POSITION) {

		//posarlo a dintre duna llista per més tard fer les validacions, una vegada fetes les validacions es borra la llista de moviments acumulats
		if (partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end()) {
			if (partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.empty()) { //es el primer paquet del client, per tant no acumulem, si no que inicialitzem el mapa
				partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, tryaccum));
			}
			else { //hem d'acumular amb l'anterior paquet accum, posar el id move del ultim, sumar deltes i posicio del ultim
				AccumMovements temp;
				temp.delta.x = partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.rbegin()->second.delta.x += tryaccum.delta.x;
				temp.delta.y = partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.rbegin()->second.delta.y += tryaccum.delta.y;
				temp.absolute = tryaccum.absolute;
				partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.erase(partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.rbegin()->first); //borrem anterior
				partidas[gID]->jugadors.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, temp)); //insertem amb els nous valors actualitzats
			}
		}

	}
	else if (TRY_COLLISION_OPPONENT) {
		if (partidas[gID]->jugadors.find(idOpponentCollision) != partidas[gID]->jugadors.end() && partidas[gID]->jugadors.find(cID) != partidas[gID]->jugadors.end()) {
			if (partidas[gID]->jugadors.find(idOpponentCollision)->second.pos.x <= partidas[gID]->jugadors.find(cID)->second.pos.x + 15 && partidas[gID]->jugadors.find(idOpponentCollision)->second.pos.x >= partidas[gID]->jugadors.find(cID)->second.pos.x - 15 && partidas[gID]->jugadors.find(idOpponentCollision)->second.pos.y <= partidas[gID]->jugadors.find(cID)->second.pos.y + 15 && partidas[gID]->jugadors.find(idOpponentCollision)->second.pos.y >= partidas[gID]->jugadors.find(cID)->second.pos.y - 15) {
				//std::cout << "Collision With Opponent " << idOpponentCollision << "  " << cID << std::endl;
				if (!partidas[gID]->jugadors[idOpponentCollision].laPara && partidas[gID]->jugadors[cID].laPara) {
					partidas[gID]->jugadors[idOpponentCollision].laPara = true;
					NotifyOtherClients(QUI_LA_PILLA, idOpponentCollision, "", gID, 0);
				}
				else if (partidas[gID]->jugadors[idOpponentCollision].laPara && !partidas[gID]->jugadors[cID].laPara) {
					partidas[gID]->jugadors[cID].laPara = true;
					NotifyOtherClients(QUI_LA_PILLA, cID, "", gID, 0);
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
	int32_t gID = 0;
	int32_t packetIDRecived = 0;
	int cmd;
	std::string mail = "";
	std::string nickname = "";
	std::string password = "";
	std::string msg = "";
	int32_t maxPlayers = 0;
	std::string namePartida = "";
	std::string passwordPartida = "";
	AccumMovements tryaccum = AccumMovements{ Position{ 0, 0 },  Position{ 0, 0 } };
	int32_t idMovements = 0;
	int32_t idOpponentCollision = 0;

	statusServer = socketServer.receive(packet, senderIP, senderPort);

	if (statusServer == sf::Socket::Done) {
		float rndPacketLoss = GetRandomFloat();
		if(rndPacketLoss < PERCENT_PACKETLOSS){
			std::cout << "Paquet perdut" << std::endl;
		}
		else {
			packet >> cmd >> packetIDRecived;
			if (cmd == LOGIN) {// canviar per JOIN o Create
				packet >> nickname >> password;
			}
			else if (cmd == SIGNUP) {
				packet >> mail >> nickname >> password;
			}
			else {
				packet >> IDClient;
				if (cmd == GLOBAL_CHAT) {
					packet >> msg;
				}
				else if (cmd == NEW_GAME) {
					packet >> namePartida >> passwordPartida >> maxPlayers;
				}
				else {
					packet >> gID;
					if (cmd == GAME_CHAT) {
						packet >> msg;
					}
					else if (cmd == JOIN_GAME) {
						packet >> passwordPartida;
					}
					else if (cmd == TRY_POSITION) {
						packet >> idMovements >> tryaccum;
						//packet >> tryaccum;
					}
					else if (cmd == TRY_COLLISION_OPPONENT) {
						packet >> idOpponentCollision;
					}
				}	
			}
			ManageReveivedData(cmd, IDClient, gID, packetIDRecived, senderIP, senderPort, nickname, mail, password, msg, namePartida, passwordPartida, maxPlayers, idMovements, tryaccum, idOpponentCollision);
		}
		
	}
	//packet.clear();
}


void ControlServidor()
{
	// bind the socket to a port
	statusServer = socketServer.bind(PORT);
	socketServer.setBlocking(false);
	if (statusServer != sf::Socket::Done)
	{
		socketServer.unbind();
		//exit(0);
		std::cout << "hola" << std::endl;
		system("pause");
		
	}
	std::cout << "Server is listening to port " << PORT << ", waiting for clients " << std::endl;

}


void ManagePing() {

	//cada certa quantiat de temps enviar missatge ping
	//LOBBY
	if (clockPingLobby.getElapsedTime().asMilliseconds() > _PING) {
		SendToAllClients(PING_LOBBY, 0);
		clockPingLobby.restart();
	}
	//cada certa quantiat de temps enviar missatge ping
	//PARTIDA
	for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		if(it->second->clockPing.getElapsedTime().asMilliseconds() > _PING){
			SendToAllClients(PING, it->first);
			it->second->clockPing.restart();
		}
	}
	//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat i el borrem de la llista del server
	//LOBBY
	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
			//NotifyOtherClients(DISCONNECTION, clientes->first, "", 0);
			clientes->second.connected = false;
		}
	}

	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end();) {
		if (!clientes->second.connected) {
			std::cout << "Client " << std::to_string(clientes->first) << " disconnected." << std::endl;
			clientes = clientsOnLobby.erase(clientes);
		}
		else ++clientes;
	}

	//quan enviem el missatge ping també comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	//PARTIDA
	for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		for (std::map<int32_t, Player>::iterator clientes = it->second->jugadors.begin(); clientes != it->second->jugadors.end(); ++clientes) {
			if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
				NotifyOtherClients(DISCONNECTION, clientes->first, "", it->first, 0);
				clientes->second.connected = false;
				if (it->second->pillados.size() == 1 && clientes->second.laPara) {
					it->second->pillados.erase(clientes->first);
					PilladorRandom(it->first);
				}
			}
		}

	}
	for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		for (std::map<int32_t, Player>::iterator clientes = it->second->jugadors.begin(); clientes != it->second->jugadors.end();) {
			if (!clientes->second.connected) {
				std::cout << "Client " << std::to_string(clientes->first) << " disconnected." << std::endl;
				clientes = it->second->jugadors.erase(clientes);
			}
			else ++clientes;
		}
	}
}

int main()
{
	ControlServidor();
	clockPingLobby.restart();
	clockSendLobby.restart();

	myWalls = new Walls();
	//for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) it->second->clockSend.restart();
	while (true) {
		ReceiveData();
		ManagePing();
		//cada certa quantiat de temps enviar missatges
		//LOBBY
		if (clockSendLobby.getElapsedTime().asMilliseconds() > SENDING_PING) {
			ResendLobby();
			clockSendLobby.restart();
		}
		//PARTIDES
		//for (std::map<int32_t, Partida*>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		//	if (it->second->jugadors.size() >= it->second->maxPlayers) SendToAllClients(GAME_DELETED, it->first);
		//	Winner(it->first);
		//	if (it->second->clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS) {
		//		PositionValidations(it->first);
		//		it->second->clockPositions.restart();
		//	}

		//	//cada certa quantiat de temps enviar missatges
		//	/*if (it->second->clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
		//		Resend(it->first);
		//		it->second->ComprovacioPillats();
		//		it->second->clockSend.restart();
		//	}*/
		//	//if (it->second->jugadors.size() >= it->second->maxPlayers && !it->second->gameStarted) {
		//	//	//game starts!
		//	//	it->second->gameStarted = true;
		//	//	SendToAllClients(GAMESTARTED, it->first);
		//	//}
		//	//if (it->second->gameStarted && !it->second->once) {
		//	//	PilladorRandom(it->first);
		//	//	it->second->once = true;
		//	//}

		//}
	}

	socketServer.unbind();
	system("pause");
	return 0;
}

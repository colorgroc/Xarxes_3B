//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include<GlobalValues.h>
#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <cppconn\driver.h>
#include <cppconn\resultset.h>
#include <cppconn\statement.h>
#include <cppconn\exception.h>

std::map<int32_t, ClientLobby> clientsOnLobby;
int32_t packetIDLobby = 1;
sf::Clock clockPingLobby, clockSendLobby;
int32_t idPartida = 1;

#define HOST "tcp://192.168.1.40:3306"
#define user "root"
#define pwd "annamarc"
#define shema "gamebd" //nom base de dades


bool online = true;

std::mutex myMutex;
std::map<int32_t, Partida> partidas;
sf::UdpSocket socketServer;
sf::Socket::Status statusServer;
int32_t clientID = 1;
Walls * myWalls;
//bool online = true;

class DBManager {
private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	
public:
	DBManager() {
		driver = get_driver_instance();
		con = driver->connect(HOST, user, pwd);
		stmt = con->createStatement();
		stmt->execute("USE gamedb");
	};
	~DBManager() {
		delete stmt;
		delete con;
	};

	bool Register(std::string param1, std::string param2, std::string param3) {
		sql::ResultSet* resultSet;
		std::string cmd = "SELECT count(*) FROM Players WHERE UserName=";
		cmd += "'" + param1 + "'";
		//std::string cmd = "SELECT count(*) FROM Players WHERE UserName='param1'";
		resultSet = stmt->executeQuery(cmd.c_str());
		if (resultSet->next()) {
			int num = resultSet->getInt(1);
			delete resultSet;
			if (num == 0) {
				cmd.clear();
				std::cout << "signed in" << std::endl;
				cmd = "INSERT into Players (UserName, UserPassword, UserMail) VALUES (";
				cmd += "'" + param1 + "'," + "'" + param2 + "'," + "'" + param3 + "')";
				stmt->execute(cmd.c_str());
				std::cout << cmd << std::endl;
				return true;
			}
		}
		return false;
	};
	bool Login(std::string param1, std::string param2) {
		sql::ResultSet* resultSet;
		std::string cmd = "SELECT count(*) FROM Players WHERE UserName=";
		cmd += "'" + param1 + "'" + " and UserPassword=" + "'" + param2 + "'";
		resultSet = stmt->executeQuery(cmd.c_str());
		if (resultSet->next()) {
			int num = resultSet->getInt(1);	
			delete resultSet;
			if (num == 1) {
				std::cout << "yap" << std::endl;
				return true;
				/*sql::ResultSet* resultSet;
				resultSet = stmt->executeQuery("SELECT UserID FROM Players WHERE UserName='param1' and UserPassword='param2'");
				if (resultSet->next()) {
					int id = resultSet->getInt(1);
					stmt->execute("INSERT INTO Sessions(PlayerID, Begin) VALUES('id', CURRENT_TIMESTAMP)");
					delete resultSet;
					return true;
				}*/
			}
		}
		return false;
	}
	int ReturnPlayerID(std::string param1) {
		/*sql::ResultSet* resultSet;
		std::string cmd = "SELECT UserID FROM Players WHERE UserName=";
		cmd += "'" + param1 + "'";
		resultSet = stmt->executeQuery(cmd.c_str());
		if (resultSet->next()) {
			return resultSet->getInt(1);
		}*/
		
		std::string cmd = "SELECT UserID(";
		cmd += "'" + param1 + "')";
		stmt->execute(cmd.c_str());
		return stmt->execute("GO");
	}
	void AddSession(int idSession) {

	}

	void CloseSession(int idSession) {

	}
};

DBManager manager;

void Resend(int32_t gID) {

	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	if (partidas.find(gID) != partidas.end()) {
		for (std::map<int32_t, Player>::iterator clientes = partidas[gID].jugadors.begin(); clientes != partidas[gID].jugadors.end(); ++clientes) {
			for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
				statusServer = socketServer.send(msg->second, clientes->second.ip, clientes->second.port);
				if (statusServer == sf::Socket::Error)
					std::cout << "Error sending the message.Server to Client." << std::endl;
				/*else if (statusServer == sf::Socket::Disconnected) {
					std::cout << "Error sending the message. Client disconnected." << std::endl;
					partidas[gID]->jugadors.erase(clientes);
				}*/
				/*else if ++clientes;*/
			}
		}
	}
}
void ResendLobby() {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end();++clientes) {
		for (std::map<int32_t, sf::Packet>::iterator msg = clientes->second.resending.begin(); msg != clientes->second.resending.end(); ++msg) {
			statusServer = socketServer.send(msg->second, clientes->second.ip, clientes->second.port);
			if (statusServer == sf::Socket::Error)
				std::cout << "Error sending the message.Server to Client." << std::endl;
			/*else if (statusServer == sf::Socket::Disconnected) {
				std::cout << "Error sending the message. Client disconnected." << std::endl;
				clientsOnLobby.erase(clientes);
			}*/
			//else ++clientes;
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
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID].jugadors.begin(); clientToSend != partidas[gID].jugadors.end(); ++clientToSend)
			{
				sf::Packet packet;
				packet << cmd; //no hace falta poner packetID 
				statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending PING to client " << std::to_string(clientToSend->first) << std::endl;
				}
			}
		}
	}
	else if (cmd == GAMESTARTED) { //no es critic
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID].jugadors.begin(); clientToSend != partidas[gID].jugadors.end(); ++clientToSend)
			{
				sf::Packet packet;
				packet << cmd; //no hace falta poner packetID 
				statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending GAMESTARTED to client " << std::to_string(clientToSend->first) << std::endl;
				}
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

	if (cmd == NEW_GAME_CREATED) { 
		//if (clientsOnLobby.find(cID) != clientsOnLobby.end()) {
			for (std::map<int32_t, ClientLobby>::iterator it = clientsOnLobby.begin(); it != clientsOnLobby.end(); ++it)
			{
				sf::Packet packet;
				//if (it->first != cID) {
					packet << cmd << packetIDLobby << gID << msg << maxPlayers;// << clientsOnLobby.find(cID)->second.nickname;
					it->second.resending.insert(std::make_pair(packetIDLobby, packet));
				//}
			} packetIDLobby++;
		//}
	}
	
	if (cmd == NEW_CONNECTION) {
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << partidas[gID].packetID << cID << partidas[gID].jugadors.find(cID)->second.nickname << partidas[gID].jugadors.find(cID)->second.pos;
					it->second.resending.insert(std::make_pair(partidas[gID].packetID, packet));
				}
			} partidas[gID].packetID++;
		}
	}
	else if (cmd == DISCONNECTION) {
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it)
			{
				sf::Packet packet;
				packet << cmd;
				if (it->first != cID) {
					packet << partidas[gID].packetID << cID;
					it->second.resending.insert(std::make_pair(partidas[gID].packetID, packet));
				}
			}partidas[gID].packetID++;
		}

	}

	else if (cmd == GAME_CHAT) { //no es critic
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator clientToSend = partidas[gID].jugadors.begin(); clientToSend != partidas[gID].jugadors.end(); ++clientToSend)
			{
				if (clientToSend->first != cID) {
					sf::Packet packet;
					std::string nick = partidas[gID].jugadors.find(cID)->second.nickname;
					packet << cmd << nick << msg; //no hace falta poner packetID 
					statusServer = socketServer.send(packet, clientToSend->second.ip, clientToSend->second.port);
					if (statusServer != sf::Socket::Done) {
						std::cout << "Error sending GAME_CHAT to client " << std::to_string(clientToSend->first) << std::endl;
					}
				}
			}
		}
	}
	else if (cmd == REFRESH_POSITIONS) {
		if (partidas.find(gID) != partidas.end() && partidas[gID].jugadors.find(cID) != partidas[gID].jugadors.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it)
			{
				sf::Packet packet;
				if (it->first != cID) {
					packet << cmd << partidas[gID].packetID << cID << partidas[gID].jugadors.find(cID)->second.pos;
					statusServer = socketServer.send(packet, it->second.ip, it->second.port);
					if (statusServer != sf::Socket::Done) {
						std::cout << "Error sending REFRESH_POSITIONS to client " << std::to_string(cID) << std::endl;
					}
				}
			} partidas[gID].packetID++;
		}
	}
	else if (cmd == QUI_LA_PILLA) {
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it)
			{
				sf::Packet packet;
				packet << cmd;
				packet << partidas[gID].packetID << cID;
				it->second.resending.insert(std::make_pair(partidas[gID].packetID, packet));
			}partidas[gID].packetID++;
		}
	}
	else if (cmd == WINNER) {
		if (partidas.find(gID) != partidas.end()) {
			for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it)
			{
				sf::Packet packet;
				packet << cmd;
				packet << partidas[gID].packetID << cID;
				it->second.resending.insert(std::make_pair(partidas[gID].packetID, packet));
			}partidas[gID].packetID++;
		}
	}
}

void PilladorRandom(int32_t gID) {
	if (partidas.find(gID) != partidas.end()) {
		std::uniform_int_distribution<int32_t> num(1, partidas[gID].jugadors.size());
		std::random_device rd;
		std::mt19937 gen(rd());
		int32_t pillador = num(gen);

		for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it) {
			if (it->second.id == pillador) {
				it->second.laPara = true;
				std::cout << "La pilla el client amb nickname: " << it->second.nickname << " i amb ID: " << std::to_string(it->first) << std::endl;
				NotifyOtherClients(QUI_LA_PILLA, it->second.id, "", gID, 0);
			}
		}
	}
}

void PositionValidations(int32_t gID) {
	//recorre tota la llista de clients
	//anar validant i descartant
	//enviar amb OkPosition
	//reenviar a tots els altres jugadors
	//REFRESH_POSITIONS 
	if (partidas.find(gID) != partidas.end()) {
		for (std::map<int32_t, Player>::iterator client = partidas[gID].jugadors.begin(); client != partidas[gID].jugadors.end(); ++client) {

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
					if (partidas[gID].jugadors.size() > 1) {
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
}

void Winner(int32_t gID) {
	if (partidas.find(gID) != partidas.end() && partidas[gID].gameStarted && partidas[gID].pillados.size() >= (partidas[gID].jugadors.size() - 1)) {
		for (std::map<int32_t, Player>::iterator it = partidas[gID].jugadors.begin(); it != partidas[gID].jugadors.end(); ++it) {
			if (!it->second.laPara) {
				it->second.winner = true;
				NotifyOtherClients(WINNER, it->first, it->second.nickname, gID, 0);
			}
		}
	}
}

void ManageReveivedData(int cmd, int32_t cID, int32_t gID, int32_t pID, sf::IpAddress senderIP, unsigned short senderPort, std::string nickname, std::string mail, std::string password, std::string msg, std::string namePartida, std::string passwordPartida, int32_t maxPlayers, int32_t idMovements, AccumMovements tryaccum, int32_t idOpponentCollision) {

	if (cmd == ACK_PING_LOBBY) {
		if (clientsOnLobby.find(cID) != clientsOnLobby.end())
			clientsOnLobby.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == ACK_PING) {
		if (partidas[gID].jugadors.find(cID) != partidas[gID].jugadors.end())
			partidas[gID].jugadors.find(cID)->second.timeElapsedLastPing.restart();
	}
	else if (cmd == LOGIN) { 
		//comprobar si el client ja esta connectat. 

		if (manager.Login(nickname, password)) {
			sf::Packet packet;
			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			packet << ACK_LOGIN << pID << clientID << partidas.size();
			if (partidas.size() > 0) {
				for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
					packet << it->first;
					packet << it->second.name;
					packet << it->second.jugadors.size();
					packet << it->second.maxPlayers;
				}
			}
			clientsOnLobby.insert(std::make_pair(clientID, ClientLobby{ clientID, nickname, senderIP, senderPort, 1, 0, 0, true }));
			clientsOnLobby[clientID].timeElapsedLastPing.restart();
			clientsOnLobby[clientID].timeElapsedLastPing.restart();

			clientID++;

			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_LOGIN to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}
		else {
			sf::Packet packet;
			packet << OUT << pID;
			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_LOGIN to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}

	}
	else if (cmd == SIGNUP) { 
	

		if (manager.Register(nickname, password, mail)){ //no accepta els valors...si poso "anna", "pass" i "mail" escriu be a la base de dades

			sf::Packet packet;
			//pillar la  base ID del client a la base d dades;
			std::cout << "packet id: " << pID << std::endl;
			int32_t woa = partidas.size();
			std::cout << "size: " << woa << std::endl;
			packet << ACK_SIGNUP << pID <<  clientID << woa;
			if (partidas.size() > 0) {
				for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
					int32_t idi = it->second.id;
					//packet << it->first;
					//packet << it->second.name;
					int32_t c = it->second.jugadors.size();
					std::string n = it->second.name;
					int32_t m = it->second.maxPlayers;
					packet << idi << n << c << m;
					/*
					packet << it->second.maxPlayers;*/
				}
	
			}
			//std::cout << "manager id: " << manager.ReturnPlayerID(nickname) << std::endl;
			clientsOnLobby.insert(std::make_pair(clientID, ClientLobby{ clientID, nickname, senderIP, senderPort, 1, 0, 0, true }));
			//AFEGIR A LA BASE DE DADES
			std::cout << "Connection with client " << std::to_string(clientID) << " from PORT " << senderPort << std::endl;
			//NotifyOtherClients(NEW_CONNECTION, clientID);
			clientsOnLobby[clientID].timeElapsedLastPing.restart();
			std::cout << "client id" << clientID << std::endl;
			
			clientID++;
			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_SIGNUP to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}
		else {
			sf::Packet packet;
			packet << OUT << pID;
			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending ACK_LOGIN to client " << std::to_string(clientID - 1) << std::endl;
			}
			packet.clear();
		}
		//else enviar que no existeix 
	}
	else if (cmd == GLOBAL_CHAT || cmd == GAME_CHAT) {
		NotifyOtherClients(cmd, cID, msg, gID, 0);
	}
	else if (cmd == NEW_GAME) {
		bool create = false;
		for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
			if (it->second.name == namePartida) create = true;
		}
		if (!create) {
			Position pos;
			int32_t numOfOpponents = 0;
			srand(time(NULL));
			//std::cout << "CREATE" << std::endl;
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

			packet << WELCOME << pID << idPartida << pos << numOfOpponents;
			partidas.insert(std::make_pair(idPartida, Partida(idPartida, cID, namePartida, passwordPartida, maxPlayers)));

			partidas[idPartida].jugadors.insert(std::make_pair(cID, Player{ cID, nickname, pos, senderIP, senderPort, true, false, false }));
			statusServer = socketServer.send(packet, senderIP, senderPort);
			if (statusServer != sf::Socket::Done) {
				std::cout << "Error sending WELCOME to client " << std::to_string(clientID - 1) << std::endl;
			}
			partidas.find(idPartida)->second.clockSend.restart();
			partidas.find(idPartida)->second.clockPing.restart();
			partidas.find(idPartida)->second.jugadors.find(cID)->second.timeElapsedLastPing.restart();
			NotifyOtherClients(NEW_GAME_CREATED, cID, namePartida, idPartida, maxPlayers);
			idPartida++;
			packet.clear();
		}

	}

	else if (cmd == JOIN_GAME && partidas.find(gID) != partidas.end() && partidas[gID].jugadors.size() < partidas[gID].maxPlayers) {
																 
		bool correctPassword = true; //per ara deixarla a true
		//COMPROVAR SI LA PASSWORD ES CORRECTE SI NO HO �S, ENVIAR AL JUGADOR PASSWORD_INCORRECT --> BASE DE DADES <-- si es correcte, correctPassword = true
		std::cout << "JOIN" << std::endl;
		//si aquest usuari ja esta jugant
		//if (partidas.find(gID) != partidas.end()) {

			if (correctPassword) {
				Position pos;
				int32_t numOfOpponents = partidas.find(gID)->second.jugadors.size();
				srand(time(NULL));

				//random range c++11 stuff
				do {
					std::random_device rdX, rdY;
					std::mt19937 genX(rdX()), genY(rdY());
					std::uniform_int_distribution<int16_t> num(1, NUMBER_ROWS_COLUMNS - 2);
					pos.x = num(genX);
					pos.y = num(genY);

				} while (!myWalls->CheckCollision(pos) && !partidas.find(gID)->second.CheckCollisionWithClientsPos(pos));

				pos = CellToPixel(pos.x, pos.y);
				sf::Packet packet;
				partidas.find(gID)->second.jugadors.insert(std::make_pair(cID, Player{ cID, nickname, pos, senderIP, senderPort, true, false, false }));
				partidas.find(gID)->second.jugadors.find(cID)->second.timeElapsedLastPing.restart();
				packet << WELCOME << pID << gID << pos << numOfOpponents;
				if (numOfOpponents > 0) {
					//inserim al packet la ID i la pos de cada oponent
					for (std::map<int32_t, Player>::iterator it = partidas.find(gID)->second.jugadors.begin(); it != partidas.find(gID)->second.jugadors.end(); ++it) { //FALLA AQUI
						int32_t id = it->second.id;
						std::string nick = it->second.nickname;
						Position posu = it->second.pos;
						packet << id << nick << posu;
					}
				}
				
				//ACTUALITZAR BASE DE DADES del jugador --> partides jugades

				NotifyOtherClients(NEW_CONNECTION, cID, "", gID, 0);

				statusServer = socketServer.send(packet, senderIP, senderPort);
				if (statusServer != sf::Socket::Done) {
					std::cout << "Error sending WELCOME to client " << std::to_string(clientID - 1) << std::endl;
				}
				//packet.clear();
			}
		//}
	}

	//else if (cmd == ACK_NEW_CONNECTION_LOBBY || cmd == ACK_DISCONNECTION_LOBBY) {
	//	if (clientsOnLobby.find(cID) != clientsOnLobby.end() && clientsOnLobby[cID].resending.find(pID) != clientsOnLobby[cID].resending.end()) {
	//		clientsOnLobby[cID].resending.erase(pID);
	//	}
	//}

	else if (cmd == ACK_NEW_CONNECTION || cmd == ACK_DISCONNECTION || cmd == ACK_QUI_LA_PILLA || cmd == ACK_WINNER) {
		if (partidas.find(gID)!= partidas.end() && partidas[gID].jugadors.find(cID) != partidas[gID].jugadors.end() && partidas[gID].jugadors[cID].resending.find(pID) != partidas[gID].jugadors[cID].resending.end()) {
			partidas[gID].jugadors[cID].resending.erase(pID);
			if (cmd == ACK_WINNER) {
				partidas[gID].receivedWinner++;
			}
		}
	}	
	
	else if (cmd == TRY_POSITION) {

		//posarlo a dintre duna llista per m�s tard fer les validacions, una vegada fetes les validacions es borra la llista de moviments acumulats
		if (partidas.find(gID) != partidas.end() && partidas[gID].jugadors.find(cID) != partidas[gID].jugadors.end()) {
			if (partidas[gID].jugadors.find(cID)->second.MapAccumMovements.empty()) { //es el primer paquet del client, per tant no acumulem, si no que inicialitzem el mapa
				partidas[gID].jugadors.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, tryaccum));
			}
			else { //hem d'acumular amb l'anterior paquet accum, posar el id move del ultim, sumar deltes i posicio del ultim
				AccumMovements temp;
				temp.delta.x = partidas[gID].jugadors.find(cID)->second.MapAccumMovements.rbegin()->second.delta.x += tryaccum.delta.x;
				temp.delta.y = partidas[gID].jugadors.find(cID)->second.MapAccumMovements.rbegin()->second.delta.y += tryaccum.delta.y;
				temp.absolute = tryaccum.absolute;
				partidas[gID].jugadors.find(cID)->second.MapAccumMovements.erase(partidas[gID].jugadors.find(cID)->second.MapAccumMovements.rbegin()->first); //borrem anterior
				partidas[gID].jugadors.find(cID)->second.MapAccumMovements.insert(std::make_pair(idMovements, temp)); //insertem amb els nous valors actualitzats
			}
		}

	}
	else if (TRY_COLLISION_OPPONENT) {
		if (partidas.find(gID) != partidas.end() && partidas[gID].jugadors.find(idOpponentCollision) != partidas[gID].jugadors.end() && partidas[gID].jugadors.find(cID) != partidas[gID].jugadors.end()) {
			if (partidas[gID].jugadors.find(idOpponentCollision)->second.pos.x <= partidas[gID].jugadors.find(cID)->second.pos.x + 15 && partidas[gID].jugadors.find(idOpponentCollision)->second.pos.x >= partidas[gID].jugadors.find(cID)->second.pos.x - 15 && partidas[gID].jugadors.find(idOpponentCollision)->second.pos.y <= partidas[gID].jugadors.find(cID)->second.pos.y + 15 && partidas[gID].jugadors.find(idOpponentCollision)->second.pos.y >= partidas[gID].jugadors.find(cID)->second.pos.y - 15) {
				//std::cout << "Collision With Opponent " << idOpponentCollision << "  " << cID << std::endl;
				if (!partidas[gID].jugadors[idOpponentCollision].laPara && partidas[gID].jugadors[cID].laPara) {
					partidas[gID].jugadors[idOpponentCollision].laPara = true;
					NotifyOtherClients(QUI_LA_PILLA, idOpponentCollision, "", gID, 0);
				}
				else if (partidas[gID].jugadors[idOpponentCollision].laPara && !partidas[gID].jugadors[cID].laPara) {
					partidas[gID].jugadors[cID].laPara = true;
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
			packet >> cmd;
			if (cmd == GAME_CHAT) {
				packet >> IDClient >> gID >> msg;
			}
			else {
				packet >> packetIDRecived;
				if (cmd == LOGIN) {// canviar per JOIN o Create
					packet >> nickname >> password;
				}
				else if (cmd == SIGNUP) {
					std::cout << "packet id:" << packetIDRecived << std::endl;
					packet >> mail >> nickname >> password;
				}

				else {
					packet >> IDClient;
					/*if (cmd == GLOBAL_CHAT) {
						packet >> msg;
					}*/
					if (cmd == NEW_GAME) {
						//std::string maxi = "";
						packet >> namePartida >> passwordPartida >> maxPlayers;
						//maxPlayers = std::atoi(maxi.c_str());
						//std::cout << maxi << ", " << maxPlayers << std::endl;
					}
					else {
						packet >> gID;
						/*if (cmd == GAME_CHAT) {
							packet >> msg;
						}*/
						if (cmd == JOIN_GAME) {
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
	if (clockPingLobby.getElapsedTime().asMilliseconds() > _PING_LOBBY) {
		SendToAllClients(PING_LOBBY, 0);
		clockPingLobby.restart();
	}
	//cada certa quantiat de temps enviar missatge ping
	//PARTIDA
	for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		if(it->second.clockPing.getElapsedTime().asMilliseconds() > _PING){
			SendToAllClients(PING, it->first);
			it->second.clockPing.restart();
		}
	}
	//quan enviem el missatge ping tamb� comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat i el borrem de la llista del server
	//LOBBY
	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end(); ++clientes) {
		if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING_LOBBY) {
			//NotifyOtherClients(DISCONNECTION, clientes->first, "", 0);
			clientes->second.connected = false;
			//std::cout << "Connected == false" << std::endl;
		}
	}

	for (std::map<int32_t, ClientLobby>::iterator clientes = clientsOnLobby.begin(); clientes != clientsOnLobby.end();) {
		if (!clientes->second.connected) {
			std::cout << "Client " << std::to_string(clientes->first) << " disconnected." << std::endl;
			clientes = clientsOnLobby.erase(clientes);
		}
		else ++clientes;
	}

	//quan enviem el missatge ping tamb� comprovem que cap dels jugadors hagi superat el temps maxim
	//si es supera el temps maxim vol dir que esta desconectat, notifiquem als altres jugadors, i el borrem de la llista del server
	//PARTIDA
	for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		for (std::map<int32_t, Player>::iterator clientes = it->second.jugadors.begin(); clientes != it->second.jugadors.end(); ++clientes) {
			if (clientes->second.timeElapsedLastPing.getElapsedTime().asMilliseconds() > CONTROL_PING) {
				NotifyOtherClients(DISCONNECTION, clientes->first, "", it->first, 0);
				clientes->second.connected = false;
				if (it->second.pillados.size() == 1 && clientes->second.laPara) {
					it->second.pillados.erase(clientes->first);
					PilladorRandom(it->first);
				}
			}
		}

	}
	for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
		for (std::map<int32_t, Player>::iterator clientes = it->second.jugadors.begin(); clientes != it->second.jugadors.end();) {
			if (!clientes->second.connected) {
				std::cout << "Client " << std::to_string(clientes->first) << " disconnected." << std::endl;
				clientes = it->second.jugadors.erase(clientes);
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
		////PARTIDES
		for (std::map<int32_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
			if (it->second.clockSend.getElapsedTime().asMilliseconds() > SENDING_PING) {
				Resend(it->first);
				it->second.ComprovacioPillats();
				it->second.clockSend.restart();
			}

			//if (it->second.jugadors.size() >= it->second.maxPlayers) SendToAllClients(GAME_DELETED, it->first);
			Winner(it->first);
			if (it->second.clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS) {
				PositionValidations(it->first);
				it->second.clockPositions.restart();
			}

			//cada certa quantiat de temps enviar missatges
			
			if (it->second.jugadors.size() >= it->second.maxPlayers && !it->second.gameStarted) {
				//game starts!
				std::cout << it->second.jugadors.size() << std::endl;
				std::cout << it->second.maxPlayers << std::endl;
				it->second.gameStarted = true;
				SendToAllClients(GAMESTARTED, it->first);
			}
			if (it->second.gameStarted && !it->second.once) {
				PilladorRandom(it->first);
				it->second.once = true;
			}

		}
	}

	socketServer.unbind();
	system("pause");
	return 0;
}

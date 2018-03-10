//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include "Game.cpp"
#include "Player.cpp"

#define MAX_CLIENTS 2

#define NEW_CONNECTION 1
#define DISCONNECTED 2

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED, GAME_HAS_FINISHED } bingo;

bool online;

int puerto = 5000;

sf::Socket::Status status;
std::mutex myMutex;

std::string textoAEnviar = "";

sf::TcpListener listener;
sf::SocketSelector selector;
std::vector<sf::TcpSocket*> clients;

Game *myGame;


void shared_cout(std::string msg) {
	
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	if (msg != "") { std::cout << (msg) << std::endl; }

}

void NotifyAllClients_ConnectedOrDisconnected(int option, sf::TcpSocket *newclient) {
	
	//cuando se conecte un nuevo cliente
	if (option == NEW_CONNECTION) {
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			if (newclient->getRemotePort() != client.getRemotePort()) {
				textoAEnviar = "MESSAGE_Se ha conectado el cliente con puerto " + std::to_string(newclient->getRemotePort()) + "\n_";

				status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
			}
		}
	}
	//cuando se desconecta un cliente
	else if (option == DISCONNECTED) {
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			if (newclient->getRemotePort() != client.getRemotePort()) {
				textoAEnviar = "MESSAGE_Se ha desconectado el cliente con puerto " + std::to_string(newclient->getRemotePort()) + "\n_";
				status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
			}
		}
	}
	
}

void SendToAllOrClientDueReceivedMsg(sf::TcpSocket *fromclient, std::string msg) {
	
	std::string delimiter = "_"; //s'utilitza aquest delimitador per separa commad del msg
	std::string command = msg.substr(0, msg.find(delimiter)); //command
	msg.erase(0, msg.find(delimiter) + delimiter.length()); //msg te el misatge sense el command

	//chat
	if (command == "MESSAGE") {
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			if (fromclient->getRemotePort() != client.getRemotePort()) {
				if (msg != "Disconnected") {
					textoAEnviar = "MESSAGE_Mensaje de " + std::to_string(fromclient->getRemotePort()) + ": " + msg + "\n_";
					status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
				}
			}
		}
	}

	if (command == "NUMBER") {
		//segons la posicio del client dins el vector s'agafa el player
		//es comprova si el numero el te a la cartilla
		//si es verdader s'envia la cartilla al jugador actualitzada
		////si es fals enviem al jugador que no es veritat i no cal actualitzar la cartilla

	}
	if (command == "LINE") {
		//segons la posicio del client dins el vector s'agafa el player
		//es comprova si el jugador ha fet linia
		//si es verdader s'envia linia a tots els jugadors
		//si es fals enviem al client que ha enviat linia que no es veritat
	}
	if (command == "BINGO") {
		//segons la posicio del client dins el vector s'agafa el player
		//es comprova si el jugador ha fet bingo
		//si es verdader s'envia bingo a tots els jugadors, al jugador que ha guanyat li donem el bote
		//si es fals enviem al client que ha enviat bingo que no es veritat
	}
}

void SendToAllOrClientDueStateGame(std::string command) {

	if (command == "READYTOPLAY_") {
		//recorrer tota la llista de clients i envia que ha començat la partida
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			
			textoAEnviar = "READYTOPLAY_GO!_";
			status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
			
		}
	}
	if (command == "BOTE_") {
		//enviar a tots els clients el bote 
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;

			textoAEnviar = "BOTE_" + std::to_string(myGame->getPot())+ "\n_";
			status = client.send(textoAEnviar.c_str(), textoAEnviar.length());

		}
	}
	if (command == "NUMBER_") {
		//enviar a tots els clients el nou numero random
	}
	if (command == "BOOK_") {
		//enviar a tots els clients la seva cartilla
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			
			for (std::vector<Player>::iterator it = myGame->players.begin(); it != myGame->players.end(); ++it)
			{
				if (it->getPlayerInfo()->getRemotePort() == client.getRemotePort()) {
					textoAEnviar = "BOOK_" + it->bookReadyToSend() + "_";
					client.send(textoAEnviar.c_str(), textoAEnviar.length());
				}
			}
		}
	}
	
}


void WaitforDataOnAnySocket() {

	// Endless loop that waits for new connections
	while (online)
	{
		// Make the selector wait for data on any socket
		if (selector.wait())
		{
			// Test the listener
			if (selector.isReady(listener))
			{
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{
					// Add the new client to the clients list
					shared_cout("Se ha conectado el cliente con puerto " + std::to_string(client->getRemotePort()));
					clients.push_back(client);

					//nova conexio, s'ha de crear un nou player local i posar-lo dintre del vector de jugadors
					Player tempPlayer(client); //restem -1 a causa de erase del vector (mirar server) //https://stackoverflow.com/questions/875103/how-do-i-erase-an-element-from-stdvector-by-index
					myGame->addNewPlayerToList(tempPlayer);

					NotifyAllClients_ConnectedOrDisconnected(NEW_CONNECTION,client);
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);
				}
				else
				{
					// Error, we won't get a new connection, delete the socket
					shared_cout("Error al recoger conexion nueva");
					delete client;
				}
			}
			else
			{
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					sf::TcpSocket& client = **it;
					if (selector.isReady(client))
					{
						// The client has sent some data, we can receive it
						char buffer[100];
						size_t bytesReceived;
						status = client.receive(buffer, 100, bytesReceived);
						if (status == sf::Socket::Done)
						{
							buffer[bytesReceived] = '\0';
							shared_cout("Mensaje del puerto " + std::to_string(client.getRemotePort()) + ": " + buffer);
							//segons missatge rebut que envio?
							SendToAllOrClientDueReceivedMsg(&client, buffer);
						}
						else if (status == sf::Socket::Disconnected)
						{
							NotifyAllClients_ConnectedOrDisconnected(DISCONNECTED, &client);
							
							shared_cout("Se a desconectado el cliente con puerto " + std::to_string(client.getRemotePort()));

							//(HACER MAS ADELANTE)
							//s'ha de borrar el client del vector de clients
							//s'ha de borrar el jugador del vector de jugadors
				

							selector.remove(client);
						}
						else
						{
							shared_cout("Error al recibir de " + std::to_string(client.getRemotePort()));
						}
					}
				}
			}
		}
	}
}

void ControlServidor()
{

	// Create a socket to listen to new connections
	status = listener.listen(puerto);
	if (status != sf::Socket::Done)
	{
		std::cout << "Error al abrir listener\n";
		exit(0);
	}
	// Add the listener to the selector
	selector.add(listener);
}



int main()
{
	online = true;
	std::cout << "Server online... \n";
	textoAEnviar = "";

	ControlServidor();
	std::thread t1(&WaitforDataOnAnySocket);

	bingo = WAIT_FOR_ALL_PLAYERS;

	//creacio de la partida, crear objecte bingo (new)
	myGame = new Game();

	do {

		switch (bingo)
		{
		case WAIT_FOR_ALL_PLAYERS:
			if (clients.size() == MAX_CLIENTS) {
				bingo = ALL_PLAYERS_CONNECTED;
			}
			//ha cada nova connexio notificar als altres jugador ja conectats (ja es fa)
			break;

		case ALL_PLAYERS_CONNECTED:
			//enviar a tots els cilentes que la partida ha començat (READYTOPLAY_)
			//treure els diners de la aposta inicial de cada jugador
			//enviar a tots els jugadors el bote total (BOTE_)
			//enviar a tots els jugadors la seva cartilla 
			// (a partir d'aqui els jugadors ja poden començar a parlar amb el servidor)
			SendToAllOrClientDueStateGame("READYTOPLAY_");
			myGame->CalculatePot();
			SendToAllOrClientDueStateGame("BOTE_");
			SendToAllOrClientDueStateGame("BOOK_");
			bingo = GAME_HAS_STARTED;
			break;

		case GAME_HAS_STARTED:
			//cada cert temps (5 segons)
			//enviem els numeros random a tots els jugadors (NUMBER_)
			//escoltem continuament els missatges de tots els jugadors i actuem en consequencia (ja es fa)
			//recorrem la llista de jugadors i si algun te la variable bingo a true es canvia l'estat del switch a GAME_HAS_FINISHED
			//bingo = GAME_HAS_FINISHED;
			break;

		case GAME_HAS_FINISHED:
			//es notifica a tots els jugadors que la partida a acabat (ja s'ha dit a tots els jugadors qui ha guanyat)
				break;
		default:
			break;
		}


	} while (online);

	system("pause");
	return 0;
}
//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 2

#define NEW_CONNECTION 1
#define DISCONNECTED 2

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } bingo;

bool online;

int puerto = 5000;

sf::Socket::Status status;
std::mutex myMutex;

std::string textoAEnviar = "";

sf::TcpListener listener;
sf::SocketSelector selector;
std::vector<sf::TcpSocket*> clients;


void shared_cout(std::string msg) {
	
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	if (msg != "") { std::cout << (msg) << std::endl; }

}

void NotifyAllClients(int option, sf::TcpSocket *newclient) {
	
	//cuando se conecte un nuevo cliente
	if (option == NEW_CONNECTION) {
		for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			sf::TcpSocket& client = **it;
			if (newclient->getRemotePort() != client.getRemotePort()) {
				textoAEnviar = "Se ha conectado el cliente con puerto " + std::to_string(newclient->getRemotePort());
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
				textoAEnviar = "Se a desconectado el cliente con puerto " + std::to_string(newclient->getRemotePort());
				status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
			}
		}
	}
	
}

void SendToAllClients(sf::TcpSocket *fromclient, std::string msg) {
	
	for (std::vector<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		sf::TcpSocket& client = **it;
		if (fromclient->getRemotePort() != client.getRemotePort()) {
			textoAEnviar = "Mensaje de " + std::to_string(fromclient->getRemotePort()) + ": " + msg;
			status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
		}
		/*else {
			textoAEnviar = "Yo: " + msg;
			status = client.send(textoAEnviar.c_str(), textoAEnviar.length());
		}*/
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
					NotifyAllClients(NEW_CONNECTION,client);
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
							//aqui reenviar info als altres clients
							SendToAllClients(&client, buffer);
						}
						else if (status == sf::Socket::Disconnected)
						{
							NotifyAllClients(DISCONNECTED, &client);
							
							shared_cout("Se a desconectado el cliente con puerto " + std::to_string(client.getRemotePort()));
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

	do {

		switch (bingo)
		{
		case WAIT_FOR_ALL_PLAYERS:
			if (clients.size() == MAX_CLIENTS) {
				bingo = ALL_PLAYERS_CONNECTED;
			}
			break;

		case ALL_PLAYERS_CONNECTED:
			break;

		case GAME_HAS_STARTED:
			break;

		default:
			break;
		}


	} while (online);

	system("pause");
	return 0;
}
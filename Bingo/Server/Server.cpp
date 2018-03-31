//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#define MAX_CLIENTS 4
#define MAX_MENSAJES 25
#define NEW_CONNECTION 1
#define DISCONNECTED 2
#define PORT 50000

using PacketID = sf::Int8;
//using ClientID = int;
//using PortNumber = unsigned short;

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED } bingo;

bool online;

int puerto = 50000;

sf::Socket::Status status;
std::mutex myMutex;

std::string textoAEnviar = "";

//sf::TcpListener listener;
std::vector<sf::UdpSocket*> clients;
sf::UdpSocket socket;
//sf::SocketSelector selector;


void shared_cout(std::string msg) {

	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	if (msg != "") { std::cout << (msg) << std::endl; }

}

void NotifyAllClients(int option, sf::UdpSocket *newclient) {

	//cuando se conecte un nuevo cliente
	if (option == NEW_CONNECTION) {
		for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			
		}
	}
	//cuando se desconecta un cliente
	else if (option == DISCONNECTED) {
		for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			
		}
	}

}

void SendToAllClients(sf::UdpSocket *fromclient, std::string msg) {

	for (std::vector<sf::UdpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		
	}
}

void thread_dataReceived() {
	
	while (true) {
		char buffer[100];
		size_t bytesReceived;
		//status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
		if (status == sf::Socket::Disconnected) {
			//chat = false;
			break;
		}
		else if (status != sf::Socket::Done)
		{
			shared_cout("Ha fallado la recepcion de datos ", false);
		}
		else {
			buffer[bytesReceived] = '\0';
			shared_cout(buffer, true); //se muestra por pantalla lo recibido
		}

	}

}

void WaitforDataOnAnySocket() {

	// Endless loop that waits for new connections
	/*while (online)
	{
		
		
	}*/
}

void ControlServidor()
{
	
	// bind the socket to a port
	status = socket.bind(PORT);
	if(status != sf::Socket::Done)
	{
		socket.unbind();
		exit(0);
	}

	

}



int main()
{




	system("pause");
	return 0;
}
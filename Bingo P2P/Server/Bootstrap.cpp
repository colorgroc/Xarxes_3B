//BINGO PEER TO PEER - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include "Game.cpp"
#include "Player.cpp"

#define NUM_PLAYERS 4
#define PUERTO 50000


struct DIRECTIONS {
	std::string IP;
	unsigned short PORT;
};

std::vector<DIRECTIONS> aPeers;

sf::TcpListener listener;
sf::Socket::Status status;

int main()
{
	std::cout << "Online..." << std::endl;

	listener.listen(PUERTO);
	sf::TcpSocket sock;

	do {
		
		DIRECTIONS dir;
	
		status = listener.accept(sock); //accepto conexio entrant

		if (status == sf::Socket::Done)
		{
			//abans de posarlo a la llista li envio la info dels altres peers si ni ha
			sf::Packet packet;
			packet << (int)aPeers.size();

			for (int i = 1; i <= aPeers.size(); i++) 
			{
				packet << aPeers[i-1].IP << aPeers[i-1].PORT;
			}
		
			sock.send(packet); //envio a el nou jugador totes les dades dels altres peers
	
			//poso el nou jugador al vector de peers
			dir.IP = sock.getRemoteAddress().toString();
			dir.PORT = sock.getRemotePort();
			aPeers.push_back(dir);

			std::cout << "Numero de peers " + std::to_string(aPeers.size()) << std::endl;
		}

	} while (aPeers.size() != NUM_PLAYERS);

	listener.close();
	system("exit");
	return 0;
}


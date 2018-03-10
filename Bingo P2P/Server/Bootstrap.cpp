﻿//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#include "Game.cpp"
#include "Player.cpp"

#define NUM_PLAYERS 2

#define PUERTO 50000


struct DIRECTIONS {
	std::string IP;
	unsigned short PORT;
};
std::vector<DIRECTIONS> aPeers;
sf::TcpListener listener;
sf::Packet packet;
sf::Socket::Status status;
int main()
{
	listener.listen(PUERTO);
	//if (aPeers.size() >= NUM_PLAYERS) listener.close(); //nose si cal
	//else {
		for (int i = 0; i < NUM_PLAYERS; i++) {
			DIRECTIONS dir;
			sf::TcpSocket* sock = new sf::TcpSocket;
			status = listener.accept(*sock);
			//sf::Packet packet;
			if (status == sf::Socket::Done)
			{
				packet << aPeers.size();
				for (int i = 0; i < aPeers.size(); i++) {
					packet << aPeers[i].IP << aPeers[i].PORT;
				}
				dir.IP = sock->getRemoteAddress().toString();
				dir.PORT = sock->getRemotePort();
				aPeers.push_back(dir);
				
				sock->send(packet);
				packet.clear();
			}
			
			sock->disconnect();	
		}
		listener.close();

	//}
	//system("pause");
	system("exit");
	return 0;
}


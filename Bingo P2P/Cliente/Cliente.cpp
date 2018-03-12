//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>

#define MAX_MENSAJES 25
#define RECEIVED 1
#define WRITED 2
#define CONNECTION 3
#define NUM_PLAYERS 3
#define PUERTO 50000

struct DIRECTIONS {
	std::string IP;
	unsigned short PORT;
};

int numPlayers;
std::vector<sf::TcpSocket*> aPeers;
sf::Socket::Status status;
sf::String mensaje;
sf::TcpSocket sock;
std::vector<DIRECTIONS> aDir;
std::vector<std::string> aMensajes;
std::string textoAEnviar = "";
size_t bSent;
bool readyToPlay;

void shared_cout(std::string msg, int option) {
	//std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		//if (received) { aMensajes.push_back("Mensaje recibido: " + msg); }
		if (option == RECEIVED || option == CONNECTION) { aMensajes.push_back(msg); }
		else if (option == WRITED) { aMensajes.push_back("Yo: " + msg); }
	}
}

sf::Socket::Status SendToAllPeers(std::string msg, size_t bSent) {
	for (std::vector<sf::TcpSocket*>::iterator it = aPeers.begin(); it != aPeers.end(); ++it)
	{
		sf::Socket::Status status;
		sf::TcpSocket& aPeer = **it;
			if (msg != "Disconnected") {
				textoAEnviar = "Mensaje de " + std::to_string(sock.getLocalPort()) + ": " + msg + "\n";
				status = aPeer.send(textoAEnviar.c_str(), textoAEnviar.length(), bSent);
				return status;
			}
	}
}


int main()
{
	
	//el peer es conecta amb el bootstrap
	status = sock.connect("localhost", PUERTO);

	if (status != sf::Socket::Done) {
		std::cout << "Error de connexion";
	}
	else {
		std::cout << "Connected" << std::endl;

		sf::Packet packet;
		status = sock.receive(packet); //rebo la info de tots els altres peers
		if (status == sf::Socket::Done) {
			
			packet >> numPlayers;

			//trec tota la info de tots els peers i la poso dintre de un vector
			for (int i = 1; i <= numPlayers; i++) {
				DIRECTIONS dir;
				packet >> dir.IP >> dir.PORT;
				aDir.push_back(dir);
			}

			//aquest peer es conecta amb tots els peers que ja hi han a la partida
			if (aDir.size() != 0) {
				for (int i = 1; i <= aDir.size(); i++)
				{
					sf::TcpSocket* sockAux = new sf::TcpSocket;

					status = sockAux->connect(aDir[i-1].IP, aDir[i-1].PORT);

					if (status == sf::Socket::Done) {
						aPeers.push_back(sockAux);
						std::cout << "Connected with " << std::to_string(sockAux->getRemotePort()) << std::endl;
					}
					else if (status == sf::Socket::Disconnected) std::cout << "Couldn't connect to " << aDir[i-1].IP << " --> Disconnected." << std::endl;
					else if (status == sf::Socket::Error) std::cout << "Couldn't connect to " << aDir[i-1].IP << " --> Error." << std::endl;
				}
			}
			else {
				std::cout << "There arent no peers yet" << std::endl;
			}
		}
	}
	
	//vaig escolatat els nous peers que envia el bootstrap i el peer shi contecta
	unsigned short temp = sock.getLocalPort(); //guardo el port local
	sock.disconnect(); //desconecto el socket amb el bootstrap
	sf::TcpListener listener;
	listener.listen(temp); //escolto

	while (aPeers.size() != NUM_PLAYERS-1) {

		sf::TcpSocket* sockAux = new sf::TcpSocket;
		status = listener.accept(*sockAux);

		if (status == sf::Socket::Done) {
			std::cout << "New Connection accepted connection with: " << std::to_string(sockAux->getRemotePort()) << std::endl;
			aPeers.push_back(sockAux);
		}
	}
	listener.close(); //una vegada tinc tots els peers ja puc tancar el listener
	//readyToPlay = true;


	if (readyToPlay) {
		sock.setBlocking(false);

		sf::RenderWindow window;
		sf::Vector2i screenDimensions(800, 600);

		window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

		sf::Font font;
		if (!font.loadFromFile("calibri.ttf"))
		{
			std::cout << "Can't load the font file" << std::endl;
		}

		mensaje = "";
		//msgs mostrados
		sf::Text chattingText(mensaje, font, 14);
		chattingText.setFillColor(sf::Color(255, 160, 0));
		chattingText.setStyle(sf::Text::Bold);

		//msg q escribes
		sf::Text text(mensaje, font, 14);
		text.setFillColor(sf::Color(0, 191, 255));
		text.setStyle(sf::Text::Italic);
		text.setPosition(0, 560);

		sf::RectangleShape separator(sf::Vector2f(800, 5));
		separator.setFillColor(sf::Color(255, 0, 0, 255));
		separator.setPosition(0, 550);

		while (window.isOpen())
		{
			sf::Event evento;

			//aqui lu dl P2P
			for (int i = 0; i < aPeers.size(); i++) {
				char buffer[100];
				size_t bytesReceived;
				status = aPeers[i]->receive(buffer, 100, bytesReceived);
				if (status == sf::Socket::Done) {
					buffer[bytesReceived] = '\0';
					shared_cout(buffer, RECEIVED);
				}
				else if (status == sf::Socket::Disconnected)
				{
					shared_cout("Servidor desconectado", CONNECTION);
					//eliminar ese socket--> aPeers.erase(i);
					window.close();
				}
			}

			while (window.pollEvent(evento))
			{
				switch (evento.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					if (evento.key.code == sf::Keyboard::Escape)
						window.close();
					else if (evento.key.code == sf::Keyboard::Return)
					{
						std::string s_mensaje;


						if (mensaje == "exit") { //puedes salir siempre que quieras
							s_mensaje = "Disconnected";
						}

						status = SendToAllPeers(s_mensaje, bSent);

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error) {
								//std::cout << "Ha fallado el envio." << std::endl;
								shared_cout("Ha fallado el envio.", CONNECTION);
							}
							else if (status == sf::Socket::Disconnected) {
								//std::cout << "Servidor desconectado" << std::endl;
								shared_cout("Servidor desconectado.", CONNECTION);
							}
							else if (status == sf::Socket::Partial) {

								while (bSent < s_mensaje.length()) {
									std::string msgRest = "";
									for (size_t i = bSent; i < s_mensaje.length(); i++) {
										msgRest = s_mensaje[i];
									}
									SendToAllPeers(s_mensaje, bSent);
								}
							}

						}
						else shared_cout(mensaje, WRITED);

						if (mensaje == "exit") {
							sock.disconnect();
							window.close();
						}


						if (aMensajes.size() > 25)
						{
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
						}
						mensaje = "";
					}
					break;

				case sf::Event::TextEntered:
					if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
						mensaje += (char)evento.text.unicode;
					else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
						mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
					break;
				}
			}
		}

		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}

		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();
	}
	
	system("pause");
	return 0;
}



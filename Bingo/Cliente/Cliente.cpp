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

int state = 1;

sf::TcpSocket socket;
std::vector<sf::TcpSocket*> aSock;

int puerto = 5000;

sf::Socket::Status status;
std::mutex myMutex;

bool chat;
std::vector<std::string> aMensajes;
sf::String mensaje;


void shared_cout(std::string msg, int option) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		//if (received) { aMensajes.push_back("Mensaje recibido: " + msg); }
		if (option == RECEIVED || option == CONNECTION) { aMensajes.push_back(msg); }
		else if(option == WRITED) { aMensajes.push_back("Yo: " + msg); }
	}
}

void NonBlockingChat() {

	status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
	if (status == sf::Socket::Error)
	{
		std::cout << "No se ha podido conectar con el servidor. Reintentelo de nuevo." << std::endl;
	}
	else if (status == sf::Socket::Disconnected)
	{
		std::cout << "Servidor desconectado." << std::endl;
		state = 0;
	}
	else {
		chat = true;
		std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
		std::cout << texto;
	}

	socket.setBlocking(false);

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

		if (chat) {
			char buffer[100];
			size_t bytesReceived;

			status = socket.receive(buffer, 100, bytesReceived); 

			if (status == sf::Socket::Done)
			{
				buffer[bytesReceived] = '\0';
				shared_cout(buffer, RECEIVED); 
			}
			else if (status == sf::Socket::Disconnected)
			{
				//std::cout << "Servidor desconectado" << std::endl;
				shared_cout("Servidor desconectado", CONNECTION); 
				chat = false;
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
					if (chat) {
						////////////////////
						std::string s_mensaje;
						size_t bSent;

					

						if (mensaje == "exit") {
							s_mensaje = "Disconnected";
						}
						else {
							s_mensaje = mensaje;
						}
						//s_mensaje = mensaje;
						status = socket.send(s_mensaje.c_str(), s_mensaje.length(), bSent);
						

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
									socket.send(msgRest.c_str(), msgRest.size(), bSent);
								}
							}
						} else shared_cout(mensaje, WRITED);

						if (mensaje == "exit") {
							chat = false;
							socket.disconnect();
						}
					}
					///////////////////

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
}

int main()
{
	std::cout << "Estableciendo conexion con server... \n";

	switch (state)
	{
	case 1:
		NonBlockingChat();
		break;
	case 0:
		socket.disconnect();
		system("pause");
		break;
	}

	return 0;
}
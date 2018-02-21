//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MENSAJES 25

int state = 1;

sf::TcpSocket socket;
std::vector<sf::TcpSocket*> aSock;

int puerto = 50000;

sf::Socket::Status status;
std::mutex myMutex;

bool chat;
std::vector<std::string> aMensajes;
sf::String mensaje;


void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		if (received) { aMensajes.push_back("Mensaje recibido: " + msg); }
		else { aMensajes.push_back(msg); }
	}
}

void thread_dataReceived() {

	while (true) {
		char buffer[100];
		size_t bytesReceived;
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
		if (status == sf::Socket::Disconnected) {
			chat = false;
			state = 0;
			//socket.disconnect();
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

void ThreadingAndBlockingChat() {

	status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
	if (status == sf::Socket::Error)
	{
		std::cout << "No se ha podido conectar. Error." << std::endl;

	}
	else if (status == sf::Socket::Disconnected) {
		std::cout << "No se ha podido conectar. Disconnected." << std::endl;
		state = 0;
		//socket.disconnect();
	}
	else {
		chat = true;
		std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
		std::cout << texto;
	}

	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente


	std::thread t1(&thread_dataReceived);
	////////////////////////////////

	sf::RenderWindow window;
	sf::Vector2i screenDimensions(800, 600);

	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = "";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(255, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


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
					////////////////////
					if (chat) {
						std::string s_mensaje;
						if (mensaje == "exit") {
							s_mensaje = "Chat finalizado";
							chat = false;
						}
						else {
							s_mensaje = mensaje;
						}
						status = socket.send(s_mensaje.c_str(), s_mensaje.length());

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error)
								shared_cout("Ha fallado el envio", false);
							if (status == sf::Socket::Disconnected) {
								shared_cout("Disconnected", false);
								state = 0;
								//socket.disconnect();
							}
						}
						else shared_cout(mensaje, false);

						if (mensaje == "exit") {
							chat = false;
							t1.join();
							state = 0;
							//socket.disconnect();

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

void NonBlockingChat() {

	status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
	if (status != sf::Socket::Done)
	{
		std::cout << "No se ha podido conectar" << std::endl;
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

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(255, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


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
				shared_cout(buffer, true); 
			}
			else if (status == sf::Socket::Disconnected)
			{
				shared_cout("Desconectado", false); 
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
						////////////////////------------------------------------------------
						std::string s_mensaje;
						size_t bSent;

						if (mensaje == "exit") {
							s_mensaje = "Chat finalizado";
						}
						else {
							s_mensaje = mensaje;
						}

						status = socket.send(s_mensaje.c_str(), s_mensaje.length(), bSent);
						

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error)
								shared_cout("Ha fallado el envio", false);
							else if (status == sf::Socket::Disconnected)
								shared_cout("Disconnected", false);
							else if (status == sf::Socket::Partial) {
								while (bSent < s_mensaje.length()) {
									std::string msgRest = "";
									for (size_t i = bSent; i < s_mensaje.length(); i++) {
										msgRest = s_mensaje[i];
									}
									socket.send(msgRest.c_str(), msgRest.size(), bSent);
								}
							}
						} else shared_cout(mensaje, false);

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
/* //INTERFICIE AMB THREADING
#include <SFML\Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>

#define MAX_MENSAJES 30

#include <SFML\Network.hpp>
#include <thread>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;
int puerto = 50000;
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
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos -- pq no bloquegi: sock.setbloquing(false)
		if (status == sf::Socket::Disconnected) {
			chat = false;
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

int main()
{
	/////////////////////////////////
	std::cout << "¿Seras servidor (s) o cliente (c)? ... " << std::endl;
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(puerto);
		if (status != sf::Socket::Done)
		{
			std::cout << "No se puede vincular con el puerto" << std::endl;
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que m'envien una connexio --> pq no bloquegi: listener.setblocking(false) --> aixo es non-blocking
		{
			std::cout << "Error al aceptar conexion" << std::endl;
		}
		else {
			chat = true;
		}

		listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
		if (status != sf::Socket::Done)
		{
			std::cout << "No se ha podido conectar" << std::endl;
		}
		else {
			chat = true;
		}

	}
	else
	{
		exit(0);
	}

	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;


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
							
							shared_cout(mensaje, false);

							if (status != sf::Socket::Done)
							{
								if(status == sf::Socket::Error)
									shared_cout("Ha fallado el envio", false);
								if (status == sf::Socket::Disconnected)
									shared_cout("Disconnected", false);

							}
							if(mensaje == "exit") {
								chat = false;
								socket.disconnect();
								t1.join();
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

*/



//--------------------------------------------------------------------------------------------------------------------------------
/*
//INTERFICIE NON BLOCKING
#include <SFML\Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>

#define MAX_MENSAJES 30

#include <SFML\Network.hpp>
#include <thread>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;
int puerto = 50000;
std::vector<std::string> aMensajes;
sf::String mensaje;

void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		if (received) { aMensajes.push_back("Mensaje recibido: " + msg); }
		else { aMensajes.push_back(msg); }
	}
}

int main()
{
	/////////////////////////////////
	std::cout << "¿Seras servidor (s) o cliente (c)? ... " << std::endl;
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(puerto);
		if (status != sf::Socket::Done)
		{
			std::cout << "No se puede vincular con el puerto" << std::endl;
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que m'envien una connexio --> pq no bloquegi: listener.setblocking(false) --> aixo es non-blocking
		{
			std::cout << "Error al aceptar conexion" << std::endl;
		}
		else {
			chat = true;
		}

		listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
		if (status != sf::Socket::Done)
		{
			std::cout << "No se ha podido conectar" << std::endl;
		}
		else {
			chat = true;
		}

	}
	else
	{
		exit(0);
	}

	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;

	////////////////////////////////
	socket.setBlocking(false);

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = "";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	while (window.isOpen())
	{
		sf::Event evento;
		if (chat) {
			char buffer[100];
			size_t bytesReceived;

			status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
			//if(status == sf::Socket::isReady)
			
			if (status == sf::Socket::Done)
			{
				buffer[bytesReceived] = '\0';
				shared_cout(buffer, true); //se muestra por pantalla lo recibido

			}
			else if (status == sf::Socket::Disconnected)
			{
				
				shared_cout("Desconectado", false); //se muestra por pantalla lo recibido
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
					////////////////////
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
						shared_cout(mensaje, false);

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error)
								shared_cout("Ha fallado el envio", false);
							else if (status == sf::Socket::Disconnected)
								shared_cout("Disconnected", false);
							else if (status == sf::Socket::Partial) {
								//copiar aqui merdeeeee
								while (bSent < textoAEnviar.size()) {
									std::string msgRest = "";
									for (size_t i = bSent; i < textoAEnviar.size(); i++) {
										msgRest = textoAEnviar[i];
									}
									socket.send(msgRest.c_str(), msgRest.size(), bSent);
								}
							}
						}
						
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
*/

//INTERFICIE SELECTORS
#include <SFML\Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>

#define MAX_MENSAJES 30

#include <SFML\Network.hpp>
#include <thread>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;
int puerto = 50000;
std::vector<std::string> aMensajes;
sf::String mensaje;

void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		if (received) { aMensajes.push_back("Mensaje recibido: " + msg); }
		else { aMensajes.push_back(msg); }
	}
}

int main()
{
	/////////////////////////////////
	
	std::cout << "¿Seras servidor (s) o cliente (c)? ... " << std::endl;
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";
	std::vector<sf::TcpSocket*> aSock;
	sf::SocketSelector ss;

	if (who == 's')
	{
		sf::TcpListener listener;
		
		status = listener.listen(puerto);

		if (status != sf::Socket::Done)
		{
			std::cout << "No se puede vincular con el puerto" << std::endl;
		}
		else {
			ss.add(listener);
			sf::TcpSocket* sock = new sf::TcpSocket;

			if (listener.accept(*sock) != sf::Socket::Done) //se queda bloquado el thread hasta que m'envien una connexio --> pq no bloquegi: listener.setblocking(false) --> aixo es non-blocking
			{
				std::cout << "Error al aceptar conexion" << std::endl;
				delete sock;
			}
			else {
				chat = true;
				aSock.push_back(sock);
				ss.add(*sock);
				std::string texto = "Conexion con ... " + (sock->getRemoteAddress()).toString() + ":" + std::to_string(sock->getRemotePort()) + "\n";
				std::cout << texto;
			}
		}

		listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		//for (int i = 0; i < aSock.size(); i++) {
			status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
			if (status != sf::Socket::Done)
			{
				std::cout << "No se ha podido conectar" << std::endl;
			}
			else {
				chat = true;
				//for (int i = 0; i < aSock.size(); i++) {
					std::string texto = "Conexion con ... " + socket.getRemoteAddress().toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
					std::cout << texto;
				//}
			}
		//}

	}
	else
	{
		exit(0);
	}

	

	////////////////////////////////
	//std::cout << aSock.size();
	
	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = "";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	

	while (window.isOpen())
	{
		sf::Event evento;
		
		if (chat) {
			if (aSock.size() > 0) {
				sf::Packet packet;
				while (ss.wait()) {
					for (int i = 0; i < aSock.size(); i++) {
						if (ss.isReady(*aSock[i]))
						{
							// The client has sent some data, we can receive it
							std::string strRec;
							status = aSock[i]->receive(packet);

							if (status == sf::Socket::Done)
							{
								packet >> strRec;
								std::cout << "He recibido " << strRec << " del puerto " << aSock[i]->getRemotePort() << std::endl;
								//break;
							}
							else if (status == sf::Socket::Disconnected)
							{
								ss.remove(*aSock[i]);
								std::cout << "Elimino el socket que se ha desconectado\n";
								chat = false;
							}
							else
							{
								std::cout << "Error al recibir de " << aSock[i]->getRemotePort() << std::endl;
							}
						}
						
					}
				}
			}
			else {
				sf::Packet packet;
				status = socket.receive(packet);
				std::string strRec;
				if (status == sf::Socket::Done)
				{
					packet >> strRec;
					shared_cout(strRec, false);
					//std::cout << "He recibido " << strRec << " del puerto " << socket.getRemotePort() << std::endl;

				}
				else if (status == sf::Socket::Disconnected)
				{
					shared_cout("Desconectado", false); //se muestra por pantalla lo recibido
					chat = false;
				}
			
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
					////////////////////
					if (chat) {

						sf::Packet packet;
						////////////////////------------------------------------------------
						std::string s_mensaje;

						if (mensaje == "exit") {
							s_mensaje = "Chat finalizado";
						}
						else {
							s_mensaje = mensaje;
						}
						packet << s_mensaje;
						if (aSock.size() > 0) {
							for (int i = 0; i < aSock.size(); i++) {
								status = aSock[i]->send(packet);
								//status = socket.send(packet);
							}
						}
						else {
							status = socket.send(packet);
						}
						shared_cout(mensaje, false);

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error)
								shared_cout("Ha fallado el envio", false);
							else if (status == sf::Socket::Disconnected)
								shared_cout("Disconnected", false);
						}

						if (mensaje == "exit") {
							chat = false;
							if (aSock.size() > 0) {
								for (int i = 0; i < aSock.size(); i++) {
									aSock[i]->disconnect();
								}
							}else
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


//--------------------------------------------------------------------------------------------------------------------------------

/*//CHAT CON BLOCKING I SELECTORS
#include <SFML\Network.hpp>
#include <iostream>
#include <list>

int puerto = 50000;

void ControlServidor()
{
	bool running = true;
	// Create a socket to listen to new connections
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(puerto);
	if (status != sf::Socket::Done)
	{
		std::cout << "Error al abrir listener\n";
		exit(0);
	}
	// Create a list to store the future clients
	std::list<sf::TcpSocket*> clients;
	// Create a selector
	sf::SocketSelector selector;
	// Add the listener to the selector
	selector.add(listener);
	// Endless loop that waits for new connections
	while (running)
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
					std::cout << "Llega el cliente con puerto: " << client->getRemotePort() << std::endl;
					clients.push_back(client);
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);
				}
				else
				{
					// Error, we won't get a new connection, delete the socket
					std::cout << "Error al recoger conexion nueva\n";
					delete client;
				}
			}
			else
			{
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					sf::TcpSocket& client = **it;
					if (selector.isReady(client))
					{
						// The client has sent some data, we can receive it
						sf::Packet packet;
						status = client.receive(packet);
						if (status == sf::Socket::Done)
						{
							std::string strRec;
							packet >> strRec;
							std::cout << "He recibido " << strRec << " del puerto " << client.getRemotePort() << std::endl;
						}
						else if (status == sf::Socket::Disconnected)
						{
							selector.remove(client);
							std::cout << "Elimino el socket que se ha desconectado\n";
						}
						else
						{
							std::cout << "Error al recibir de " << client.getRemotePort() << std::endl;
						}
					}
				}
			}
		}
	}

}

int main()
{
	std::cout << "Seras servidor (s) o cliente (c)? ... ";
	char who;
	std::cin >> who;
	std::string str = "";

	if (who == 's')
	{
		ControlServidor();
	}
	else if (who == 'c')
	{
		sf::TcpSocket socket;
		sf::Socket::Status status = socket.connect("localhost", puerto, sf::milliseconds(15.f));
		if (status != sf::Socket::Done)
		{
			std::cout << "Error al establecer conexion\n";
			exit(0);
		}
		else
		{
			std::cout << "Se ha establecido conexion\n";
		}
		
		do
		{
			std::cout << "Escribe ... ";
			std::getline(std::cin, str);
			sf::Packet packet;
			packet << str;
			status = socket.send(packet);
			if (status != sf::Socket::Done)
			{
				std::cout << "Error al enviar\n";
			}
			std::cout << std::endl;
		} while (str != "exit");
		socket.disconnect();

	}
	else
	{
		exit(0);
	}

}*/

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

	while (chat) {
		char buffer[100];
		size_t bytesReceived;
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos -- pq no bloquegi: sock.setbloquing(false)
		if (status == sf::Socket::Disconnected) {
			chat = false;
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
	

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = " >";

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
						std::string s_mensaje = mensaje;
						status = socket.send(s_mensaje.c_str(), s_mensaje.length());

						if (status != sf::Socket::Done)
						{
							aMensajes.push_back("Ha fallado el envio");
						}
					}
					///////////////////
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					mensaje = ">";
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




/*
//CHAT BLOCKING WITH THREADING (SIN INTERFAZ GRAFICA, DESPUES DE CERRAR CONEXION AUN INTENTA RECIBIR UNA VEZ)

#include <SFML\Network.hpp>
#include <iostream>
#include <thread>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;
int puerto = 50000;
bool disconnect = false;

void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	
	if (msg != "") {
		if (received) { std::cout << "Mensaje recibido: " << msg << std::endl; }
		else { std::cout << msg << std::endl; }
	}
}

void thread_dataReceived() {

	while (chat) {
		char buffer[100];
		size_t bytesReceived;
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos -- pq no bloquegi: sock.setbloquing(false)
		if (status == sf::Socket::Disconnected) {
			chat = false;
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
	shared_cout("¿Seras servidor (s) o cliente (c)? ... ", false);
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(puerto);
		if (status != sf::Socket::Done)
		{
			shared_cout("No se puede vincular con el puerto", false);
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que m'envien una connexio --> pq no bloquegi: listener.setblocking(false) --> aixo es non-blocking
		{
			shared_cout("Error al aceptar conexion", false);
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
			shared_cout("No se ha podido conectar", false);
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

	while (chat) {

		do {
			std::getline(std::cin, textoAEnviar);
			status = socket.send(textoAEnviar.c_str(), texto.length());

			if (status != sf::Socket::Done)
			{
				shared_cout("Ha fallado el envio", false);
			}
		} while (textoAEnviar != "exit");
		chat = false;
		
	}

	 //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
	socket.disconnect();
	system("pause");
	return 0;
}
*/
//--------------------------------------------------------------------------------------------------------------------------------

/*
//CHAT SIMPLE CON BLOCKING I SIN THREADS (SIN INTERFAZ GRAFICA, ENVIA UN MENAJE Y EL OTRO ESCUCHA Y ASI SUCCESIVAMENTE)

#include <SFML\Network.hpp>
#include <iostream>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;
int puerto = 50000;


void shared_cout(std::string msg, bool received) {
	
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	if (msg == "Chat finalizado") {
		chat = false;
	}
	if (msg != "") {
		if (received) { std::cout << "Mensaje recibido: " << msg << std::endl; }
		else { std::cout << msg << std::endl; }
	}
}

int main()
{
	shared_cout("¿Seras servidor (s) o cliente (c)? ... ",false);
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(puerto);
		
		shared_cout("Esperando conexion...",false);

		if (status != sf::Socket::Done)
		{
			shared_cout("No se puede vincular con el puerto",false);
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que es aceptado
		{
			shared_cout("No se ha podido establecer la conexion con " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()),false);
		}
	
		listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		shared_cout("Esperando conexion...", false);

		status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
		
		while (status != sf::Socket::Done) {
			status = socket.connect("localhost", puerto, sf::milliseconds(15.f));
		}
		

	}
	
	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	shared_cout(texto, false);
	
	socket.setBlocking(false);

	while (true) {

		char buffer[100];
		size_t bytesReceived;

		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos

		if (status == sf::Socket::NotReady) { //es queda aqui atrapat holy shit
			//std::cout << "Not Ready. " << std::endl;
			continue;
		}
		else if (status == sf::Socket::Error) {
			shared_cout("Error. ", false);
		}
		else if (status == sf::Socket::Partial) {
			shared_cout("Partial. ", false);
		}
		else if (status == sf::Socket::Done)
		{
			buffer[bytesReceived] = '\0';
			shared_cout(buffer, true); //se muestra por pantalla lo recibido

		}
		else if (status == sf::Socket::Disconnected)
			break;



		std::getline(std::cin, textoAEnviar);
		status = socket.send(textoAEnviar.c_str(), texto.length());

		if (status != sf::Socket::Done)
		{
			shared_cout("Ha fallado el envio", false);
		}


	}
	socket.disconnect(); //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
	system("pause");

	return 0;
}
*/

//--------------------------------------------------------------------------------------------------------------------------------
/*
//CHAT CON BLOCKING I SELECTORS
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

}
*/
//CHAT BLOCKING WITH THREADING (SIN INTERFAZ GRAFICA, DESPUES DE CERRAR CONEXION AUN INTENTA RECIBIR UNA VEZ)

#include <SFML\Network.hpp>
#include <iostream>
#include <thread>
#include <mutex>

sf::TcpSocket socket;
sf::Socket::Status status;
std::mutex myMutex;
bool chat;

void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex);
	if (msg == "Chat finalizado") {
		chat = false;
	}
	if (msg != "") {
		if (received) { std::cout << "Mensaje recibido: " << msg << std::endl; }
		else { std::cout << msg << std::endl; }
	}
}

void thread_dataReceived() {

	while (chat) {
		char buffer[100];
		size_t bytesReceived;
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
		if (status != sf::Socket::Done)
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
	std::string textoAEnviarBefore = "init";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(50000);
		if (status != sf::Socket::Done)
		{
			shared_cout("No se puede vincular con el puerto", false);
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que es aceptado
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
		status = socket.connect("localhost", 50000, sf::milliseconds(15.f)); //bloqueo durante un tiempo
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

		//std::cin >> textoAEnviar;
		std::getline(std::cin, textoAEnviar);

		if (textoAEnviar != textoAEnviarBefore) {
			if (textoAEnviar == "exit") {
				textoAEnviar = "Chat finalizado";
				chat = false;
			}
			status = socket.send(textoAEnviar.c_str(), texto.length());
			if (status != sf::Socket::Done)
			{
				shared_cout("Ha fallado el envio", false);
			}
			textoAEnviarBefore = textoAEnviar;
		}

	}
	t1.join();
	socket.disconnect(); //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
	system("pause");

	return 0;
}



//CHAT SIMPLE CON BLOCKING I SIN THREADS (SIN INTERFAZ GRAFICA, ENVIA UN MENAJE Y EL OTRO ESCUCHA Y ASI SUCCESIVAMENTE)
/*
#include <SFML\Network.hpp>
#include <iostream>


int main()
{
	std::cout << "¿Seras servidor (s) o cliente (c)? ... ";
	char who;
	std::cin >> who;
	std::string textoAEnviar = "";
	sf::TcpSocket socket;
	sf::Socket::Status status;

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(50000);
		if (status != sf::Socket::Done)
		{
			std::cout << "No se puede vincular con el puerto";
		}

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que es aceptado
		{
			std::cout << "Error al aceptar conexion";
		}

		listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		status = socket.connect("localhost", 50000, sf::milliseconds(15.f)); //bloqueo durante un tiempo
		if (status != sf::Socket::Done)
		{
			std::cout << "No se ha podido conectar";
		}

	}
	else
	{
		exit(0);
	}

	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;

	while (true) {
		if (who == 's') {

			char buffer[100];
			size_t bytesReceived;
			status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado la recepcion de datos ";
			}
			else {
				buffer[bytesReceived] = '\0';
				std::cout << "Mensaje recibido: " << buffer << std::endl; //se muestra por pantalla lo recibido
			}


			std::cin >> textoAEnviar;
			status = socket.send(textoAEnviar.c_str(), texto.length());
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado el envio";
			}

		}
		else if (who == 'c') {

			std::cin >> textoAEnviar;
			status = socket.send(textoAEnviar.c_str(), texto.length());
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado el envio";
			}


			char buffer[100];
			size_t bytesReceived;
			status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado la recepcion de datos ";
			}
			else {
				buffer[bytesReceived] = '\0';
				std::cout << "Mensaje recibido: " << buffer << std::endl; //se muestra por pantalla lo recibido
			}
		}
	}
	socket.disconnect(); //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
	system("pause");

	return 0;
}

*/




//PRIMERA PROVA AMB INTERFICIE, (NO FUNCIONA)
/*
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>
#define MAX_MENSAJES 30
void Server(sf::TcpSocket *socket, std::string textoAEnviar) {
	sf::Socket::Status status;
	sf::TcpListener listener;
	status = listener.listen(5000);
	status = listener.accept(*socket);
	if (status == sf::Socket::Done) {
		//std::cin >> textoAEnviar;
		textoAEnviar = "Mensaje desde servidor\n";
	}
	else if (status == sf::Socket::Error) {
		std::cout << "No se ha podido establecer la conexion. Error." << std::endl;
	}
	else if (status == sf::Socket::Disconnected) {
		std::cout << "No se ha podido establecer la conexion. Desconnectado." << std::endl;
	}
}
void Cliente(sf::TcpSocket *socket, std::string textoAEnviar, sf::IpAddress ip) {
	sf::Socket::Status status;
	status = socket->connect(ip, 5000, sf::milliseconds(15.f));
	if (status == sf::Socket::Done) {
		//std::cin >> textoAEnviar;
		textoAEnviar = "Mensaje desde cliente\n";
	}
	else if (status == sf::Socket::Error) {
		std::cout << "No se ha podido establecer la conexion. Error." << std::endl;
	}
	else if (status == sf::Socket::Disconnected) {
		std::cout << "No se ha podido establecer la conexion. Desconnectado." << std::endl;
	}
}
void Reception(sf::TcpSocket *socket, std::vector<std::string> aMsg) { //nose si esta be la veritat XD
	sf::Socket::Status status;
	while (true) {
		char buffer[MAX_MENSAJES];
		size_t bR;
		status = socket->receive(buffer, MAX_MENSAJES, bR);
		if (status == sf::Socket::Done) {
			buffer[bR] = '\0';
			std::cout << buffer << std::endl;
		}
		else if (status == sf::Socket::Error) {
			std::cout << "Error d'enviament." << std::endl;
			break;
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Usuari desconnectat." << std::endl;
			break;
		}
	}
}

void Chat(sf::TcpSocket *socket) {
	sf::Socket::Status status;
	std::vector<std::string> aMensajes;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}
	sf::String mensaje = " >";

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
					//SEND

					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					mensaje = ">";
					//Anna --> va aqui?
					/*std::string str = "";
					for (std::vector<std::string>::iterator it = aMensajes.begin(); it != aMensajes.end(); ++it) {
						str += it->c_str();
					}
					status = socket->send(str.c_str(), str.length());
					if (status == sf::Socket::Disconnected) {
						std::cout << "Usuari Desconnectat";
						exit(0);
					}
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());

				//o va aqui?
				break;
			}
		}
		window.draw(separator);
		//REECEIVE
		if (status == sf::Socket::Done) {

			//Reception(socket, aMensajes);
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
}


int main()
{
	sf::Socket::Status status;
	//sf::IpAddress ip = "192.168.122.2";
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();

	std::cout << "¿Seras servidor (s) o cliente (c)? ... ";
	char c;
	std::cin >> c;
	sf::TcpSocket socket;
	std::string textoAEnviar = "";
	if (c == 's')
	{
		Server(&socket, textoAEnviar);

	}
	else if (c == 'c')
	{

		Cliente(&socket, textoAEnviar, ip);

	}
	else
	{
		exit(0);
	}

	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;

	status = socket.send(textoAEnviar.c_str(), texto.length());

	char buffer[100];

	size_t bytesReceived;

	status = socket.receive(buffer, 100, bytesReceived);
	if (status == sf::Socket::Done) {
		buffer[bytesReceived] = '\0';
		std::cout << "Mensaje recibido: " << buffer << std::endl;
	}

	Chat(&socket);

	system("pause");

	return 0;

}*/



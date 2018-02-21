//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_CLIENTS 10

sf::TcpSocket socket;
std::vector<sf::TcpSocket*> aSock;

int puerto = 50000;

sf::Socket::Status status;
std::mutex myMutex;

bool chat;
std::string textoAEnviar = "";


void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") { //fer aqui q si el msg == tal msg, q envii noseq?
		if (received) { 
			std::cout << ("Mensaje recibido: " + msg) << std::endl; 
			textoAEnviar = msg; 
		}
		else { std::cout << (msg) << std::endl; }
	}
}

void thread_dataReceived() {

	while (true) {
		char buffer[100];
		size_t bytesReceived;
		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
		if (status == sf::Socket::Disconnected) {
			chat = false;
			//state = 0;
			socket.disconnect();
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

void SendChat() {

	std::getline(std::cin, textoAEnviar);
	if (textoAEnviar == "exit") {
		textoAEnviar = "Chat finalizado";
		chat = false;
	}

	size_t bSent;
	//fer for q recorri llista de clients conectats i reenviarelshi el msg que hem rebut excepte a qiu ens el ha enviat --> aixo ultim no cal si no imprimim el msg per pantalla al client ehehhe
	status = socket.send(textoAEnviar.c_str(), textoAEnviar.length(), bSent);

	if (status != sf::Socket::Done)
	{
		if (status == sf::Socket::Error)
			shared_cout("Ha fallado el envio", false);
		else if (status == sf::Socket::Disconnected)
			shared_cout("Disconnected", false);
		else if (status == sf::Socket::Partial) {
			while (bSent < textoAEnviar.size()) {
				std::string msgRest = "";
				for (size_t i = bSent; i < textoAEnviar.size(); i++) {
					msgRest = textoAEnviar[i];
				}
				socket.send(msgRest.c_str(), msgRest.size(), bSent);
			}
		}
	}

	if (textoAEnviar == "exit" || textoAEnviar == "Chat finalizado") {
		chat = false;
		socket.disconnect();
	}
}

void Send(std::string textoAEnviar) {

	size_t bSent;
	//fer for q recorri llista de clients conectats i reenviarelshi el msg que hem rebut excepte a qiu ens el ha enviat --> aixo ultim no cal si no imprimim el msg per pantalla al client ehehhe
	status = socket.send(textoAEnviar.c_str(), textoAEnviar.length(), bSent);

	if (status != sf::Socket::Done)
	{
		if (status == sf::Socket::Error)
			shared_cout("Ha fallado el envio", false);
		else if (status == sf::Socket::Disconnected)
			shared_cout("Disconnected", false);
		else if (status == sf::Socket::Partial) {
			while (bSent < textoAEnviar.size()) {
				std::string msgRest = "";
				for (size_t i = bSent; i < textoAEnviar.size(); i++) {
					msgRest = textoAEnviar[i];
				}
				socket.send(msgRest.c_str(), msgRest.size(), bSent);
			}
		}
	}
}



void Receive() {
	char buffer[100];
	size_t bytesReceived;

	status = socket.receive(buffer, 100, bytesReceived);

	if (status == sf::Socket::Done)
	{
		buffer[bytesReceived] = '\0';
		shared_cout(buffer, true);
		//Send(textoAEnviar);
	}
	else if (status == sf::Socket::Disconnected)
	{
		shared_cout("Desconectado", false);
		chat = false;
		//fer socket.disconnect?
	}
}

void NonBlockingChat() {
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
		std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
		std::cout << texto;
	}

	listener.close();
	socket.setBlocking(false);

	while (chat) {
		Receive();	
	}
}



int main()
{
	std::cout << "¿Server online... \n";
	textoAEnviar = "";
	//posar bool notConected?
	NonBlockingChat();

	//si notConected fer lu d sota?
	socket.disconnect();
	system("pause");
	return 0;
}
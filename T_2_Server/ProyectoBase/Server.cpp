//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_CLIENTS 10

int state = 1;
sf::TcpSocket socket;
std::vector<sf::TcpSocket*> aSock;

int puerto = 50000;

sf::Socket::Status status;
std::mutex myMutex;

bool chat;
std::string textoAEnviar = "";


void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		if (received) { std::cout<<("Mensaje recibido: " + msg)<<std::endl; }
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

	sf::TcpListener listener;
	status = listener.listen(puerto);
	if (status != sf::Socket::Done)
	{
		std::cout << "No se puede vincular con el puerto" << std::endl;
	}

	if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que m'envien una connexio
	{
		std::cout << "Error al aceptar conexion" << std::endl;
	}
	else {
		chat = true;
	}

	listener.close(); //ya no hace falta porque no hay mas solicitudes de conexion

//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;


	std::thread t1(&thread_dataReceived);

	while (chat) {
		std::getline(std::cin, textoAEnviar);
		if (textoAEnviar == "exit") {
			textoAEnviar = "Chat finalizado";
			chat = false;
		}
		
		status = socket.send(textoAEnviar.c_str(), textoAEnviar.length());

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
		if (textoAEnviar == "exit") {
			chat = false;
			t1.join();
			state = 0;
			//socket.disconnect();
		}
	}

}

int main()
{
	std::cout << "¿Server online... \n";
	textoAEnviar = "";

	switch (state)
	{
	case 1:
		ThreadingAndBlockingChat();
		break;
	case 0:
		//t1.join();
		socket.disconnect();
		system("pause");
		break;
	}
	return 0;
}

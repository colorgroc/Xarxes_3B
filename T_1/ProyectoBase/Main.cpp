/*//CHAT BLOCKING WITH THREADING (SIN INTERFAZ GRAFICA, DESPUES DE CERRAR CONEXION AUN INTENTA RECIBIR UNA VEZ)

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
	if (msg == "Chat finalizado") {
		chat = false;
		socket.disconnect();
	}
	if (msg != "") {
		if (received) { std::cout << "Mensaje recibido: " << msg << std::endl; }
		else { std::cout << msg << std::endl; }
	}
}

void thread_dataReceived() {

	while (chat) { //poner parcial
		char buffer[100];
		size_t bytesReceived;

			status = socket.receive(buffer, 100, bytesReceived); // bloquea el thread principal hasta que no llegan los datos -- pq no bloquegi: sock.setbloquing(false)
		
			if (status == sf::Socket::Error)
			{
				shared_cout("Ha fallado la recepcion de datos por Error.", false);
			}
			else if (status == sf::Socket::Disconnected) {
				//shared_cout("Ha fallado la recepcion de datos por Disconnexion.", false);
				chat = false;
				socket.disconnect();
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

			std::getline(std::cin, textoAEnviar);
		
			if (textoAEnviar != textoAEnviarBefore) {
				if (textoAEnviar == "exit") {
					textoAEnviar = "Chat finalizado";
					chat = false;
				}
				status = socket.send(textoAEnviar.c_str(), texto.length());
				if (status == sf::Socket::Error)
				{
					shared_cout("Ha fallado el envio por error de connexion.", false);
				}
				else if (status == sf::Socket::Disconnected)
				{
					//shared_cout("Ha fallado el envio por desconnexion.", false);
					chat = false;
				}
				textoAEnviarBefore = textoAEnviar;
			}
		//}
	}
	//t1.join();
	 //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
		socket.disconnect();
		system("pause");
		return 0;
}*/

//--------------------------------------------------------------------------------------------------------------------------------

//CHAT SIMPLE CON BLOCKING I SIN THREADS (SIN INTERFAZ GRAFICA, ENVIA UN MENAJE Y EL OTRO ESCUCHA Y ASI SUCCESIVAMENTE)

#include <SFML\Network.hpp>
#include <iostream>
std::string textoAEnviar = "";
sf::TcpSocket socket;

sf::Socket::Status status;

int main()
{
	std::cout << "¿Seras servidor (s) o cliente (c)? ... ";
	char who;
	std::cin >> who;
	socket.setBlocking(false);
	//std::string texto = "";

	if (who == 's')
	{
		sf::TcpListener listener;
		status = listener.listen(50000);
		
		std::cout << "Esperando conexion..." << std::endl;
		/*if (status != sf::Socket::Done)
		{
			std::cout << "No se puede vincular con el puerto";
		}*/

		if (listener.accept(socket) != sf::Socket::Done) //se queda bloquado el thread hasta que es aceptado
		{
			std::cout << "No se ha podido establecer la conexion con " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) << std::endl;
			//return -1;
		}
		else {
			listener.setBlocking(false);
			//listener.close();
		}

		
			//listener.close();
			
	

		 //ya no hace falta porque no hay mas solicitudes de conexion
	}
	else if (who == 'c')
	{
		std::cout << "Esperando conexion..." << std::endl;
		status = socket.connect("localhost", 50000, sf::milliseconds(15.f)); //bloqueo durante un tiempo
		while (status != sf::Socket::Done) {
			status = socket.connect("localhost", 50000, sf::milliseconds(15.f));
		}
		//texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
		//std::cout << texto;

		/*if (status == sf::Socket::Error)
		{
			
			std::cout << "Error en la conexion con " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) << std::endl;
			//return -1;
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Desconexion con " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) << std::endl;
		}
		else if(status == sf::Socket::Done){
			texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
			std::cout << texto;
		}
		else {
			//falla aqui
			std::cout << "Cliente no listo aun." << std::endl;
		}*/

	}
	/*do {
		if (who == 'c') {
			status = socket.connect("localhost", 50000, sf::milliseconds(15.f));
			if (status == sf::Socket::Done) {
				texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
				std::cout << texto;
			}
		}
	} while (socket.connect("localhost", 50000, sf::milliseconds(15.f)) != sf::Socket::Done);*/
	

	//se muestra por pantalla con quien se ha hecho la conexion, tanto en el server como en el cliente
	std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
	std::cout << texto;
	
	while (true) {
		//if (who == 's') {

		char buffer[100];
		size_t bytesReceived;

		status = socket.receive(buffer, 100, bytesReceived); //bloquea el thread principal hasta que no llegan los datos
		if (status == sf::Socket::NotReady) { //es queda aqui atrapat holy shit
			std::cout << "Not Ready. " << std::endl;

			/*std::getline(std::cin, textoAEnviar);
			status = socket.send(textoAEnviar.c_str(), texto.length());
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado el envio." << std::endl;
				//return -1;
			}*/
			continue;
		}
		else if (status == sf::Socket::Error) {
			std::cout << "Error. " << std::endl;
		}
		/*else if (status == sf::Socket::Partial) {
			std::cout << "Partial. " << buffer << std::endl;
		}*/
		else if (status == sf::Socket::Done)
		{
			buffer[bytesReceived] = '\0';
			std::cout << "Mensaje recibido: " << buffer << std::endl; //se muestra por pantalla lo recibido
			/*std::getline(std::cin, textoAEnviar);
			status = socket.send(textoAEnviar.c_str(), texto.length());
			if (status != sf::Socket::Done)
			{
				std::cout << "Ha fallado el envio." << std::endl;
				//return -1;
			}*/
		}
		else if (status == sf::Socket::Disconnected)
			break;

		/*std::getline(std::cin, textoAEnviar);
		status = socket.send(textoAEnviar.c_str(), texto.length());
		if (status != sf::Socket::Done)
		{
			std::cout << "Ha fallado el envio" << std::endl;
			//return -1;
		}*/
	}

		//}
		/*else if (who == 'c') {

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
		}*/
	
	socket.disconnect(); //para cerrar la conexion creada, el otro detecta que no hay conexion gracias al status
	system("pause");

	return 0;
}


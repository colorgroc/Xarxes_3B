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

enum stateGame { GAME_HASNT_STARTED, GAME_HAS_STARTED, GAME_HAS_FINISHED } bingo;

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
std::mutex myMutex;


inline bool isInteger(const std::string & s)  //https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
{
	if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

	char * p;
	strtol(s.c_str(), &p, 10);

	return (*p == 0);
}

void shared_cout(std::string msg, int option) {

	if (msg != "") {

		//netejem pantalla si hi han massa missatges
		if (aMensajes.size() >= MAX_MENSAJES) {
			aMensajes.clear();
		}

		if (option == RECEIVED) {
			//cojer el commad i mostrar un texto segun lo enviado
			std::string delimiter = "_"; //s'utilitza aquest delimitador per separar commad del msg
			size_t pos = 0;
			std::string token;
			std::vector<std::string> allcommands;
			std::vector<std::string> alldata;

			int count = 0;
			bool flipflop = false;

			std::vector<int>::iterator it;

			//per manejar multiples missatges enviats, vaig separan amb els limitadors i vaig omplint els dos vectors amb les dades agafades 
			while ((pos = msg.find(delimiter)) != std::string::npos) {
				token = msg.substr(0, pos);
				if (!flipflop) {
					allcommands.push_back(token); count++; flipflop = true;
				}
				else { alldata.push_back(token); flipflop = false; }
				msg.erase(0, pos + delimiter.length());

			}


			for (int i = 0; i < count; i++) {

				std::string command = allcommands[i];
				std::string msg = alldata[i];

				if (command == "READYTOPLAY") {
					//cambiar estat del bingo
					//mostrar per pantalla el missatge que ha començat la partida
					bingo = GAME_HAS_STARTED;
					aMensajes.push_back("The game has started. " + msg);
				}
				if (command == "BINGO") {
					//mostar que el jugador ha guanyat
					//cambiar estat del bingo a acabat
					aMensajes.push_back(msg);
					bingo = GAME_HAS_FINISHED;
				}
				else if (command == "LINE") {
					//mostar que el jugador ha fet linia
					aMensajes.push_back("Congratulations! " + msg);
				}
				else if (command == "BOTE") {
					//mostar que el jugador el bote
					aMensajes.push_back("Pot: " + msg);

				}
				else if (command == "NUMBER") {
					//mostar al jugador el nou numero
					aMensajes.push_back("New number to find: " + msg);
				}
				else if (command == "BOOK") {

				}
				else if (command == "GAMEFINISHED") {

					aMensajes.push_back(msg);
					bingo = GAME_HAS_FINISHED;
				}
				else if (command == "MESSAGE") {
					aMensajes.push_back(msg);

				}
			}


		}
		if (option == CONNECTION) { aMensajes.push_back(msg); }
		else if (option == WRITED) { aMensajes.push_back("Yo: " + msg); }
	}
}

void NonBlockingChat() {

	for (int i = 1; i <= aPeers.size(); i++) {
		aPeers[i - 1]->connect(aPeers[i - 1]->getRemoteAddress(), aPeers[i - 1]->getLocalPort());

		if (status == sf::Socket::Error)
		{
			std::cout << "No se ha podido conectar. Reintentelo de nuevo." << std::endl;
		}
		else if (status == sf::Socket::Disconnected)
		{
			std::cout << "Desconectado." << std::endl;
		}
		else {
			std::string texto = "Conexion con ... " + aPeers[i - 1]->getRemoteAddress().toString() + ":" + std::to_string(aPeers[i - 1]->getRemotePort()) + "\n";
			std::cout << texto;
			aPeers[i - 1]->setBlocking(false);
		}
	}

	sf::RenderWindow window;
	sf::Vector2i screenDimensions(400, 600);
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	mensaje = "";
	//msgs mostrados chat
	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(255, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	//msg que escribes chat
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

		//sempre escoltem, tant si ha començat el joc com si no
		if (bingo != GAME_HAS_FINISHED) {

			for (int i = 1; i <= aPeers.size(); i++) {
				char buffer[1000];
				size_t bytesReceived;
				status = aPeers[i - 1]->receive(buffer, 1000, bytesReceived);

				if (status == sf::Socket::Done)
				{
					buffer[bytesReceived] = '\0';
					shared_cout(buffer, RECEIVED);
				}
				else if (status == sf::Socket::Disconnected)
				{
					//std::cout << "Servidor desconectado" << std::endl;
					shared_cout("Servidor desconectado", CONNECTION);
					bingo = GAME_HAS_FINISHED;
				}
			}
		}



		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				bingo = GAME_HAS_FINISHED;
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape) {
					window.close();
					bingo = GAME_HAS_FINISHED;
				}

				else if (evento.key.code == sf::Keyboard::Return)
				{
					std::string s_mensaje;
					size_t bSent;

					if (mensaje == "exit") { //puedes salir siempre que quieras
						s_mensaje = "Disconnected";
					}

					if (bingo != GAME_HAS_FINISHED) {
						////////////////////
						//segons el que escriu el jugador per consola s'envia un command mes el missatge
						if (bingo == GAME_HAS_STARTED) {
							if (mensaje == "line") {
								s_mensaje = "LINE_";
								s_mensaje.append(mensaje);

							}
							else if (mensaje == "bingo") {
								s_mensaje = "BINGO_";
								s_mensaje.append(mensaje);

							}
							else if (isInteger(mensaje)) {
								s_mensaje = "NUMBER_";
								s_mensaje.append(mensaje);

							}
							else {
								s_mensaje = "MESSAGE_";
								s_mensaje.append(mensaje);
								

							}
						}
						else if (bingo == GAME_HASNT_STARTED) {
							s_mensaje = "MESSAGE_";
							s_mensaje.append(mensaje);
							s_mensaje.append("_");
						}

						for (int i = 1; i <= aPeers.size(); i++) {

							status = aPeers[i - 1]->send(s_mensaje.c_str(), s_mensaje.length(), bSent);


							if (status != sf::Socket::Done)
							{
								if (status == sf::Socket::Error) {
									//std::cout << "Ha fallado el envio." << std::endl;
									shared_cout("Ha fallado el envio.", CONNECTION);
								}
								else if (status == sf::Socket::Disconnected) {
									//std::cout << "Servidor desconectado" << std::endl;
									shared_cout("Peer desconectado.", CONNECTION);
								}
								else if (status == sf::Socket::Partial) {

									while (bSent < s_mensaje.length()) {
										std::string msgRest = "";
										for (size_t i = bSent; i < s_mensaje.length(); i++) {
											msgRest = s_mensaje[i];
										}
										aPeers[i - 1]->send(s_mensaje.c_str(), s_mensaje.length(), bSent);
									}
								}
							}
						}
						if (status == sf::Socket::Done) shared_cout(mensaje, WRITED);

						if (mensaje == "exit") {
							bingo = GAME_HAS_FINISHED;
							window.close();
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

					status = sockAux->connect(aDir[i - 1].IP, aDir[i - 1].PORT);

					if (status == sf::Socket::Done) {
						aPeers.push_back(sockAux);
						std::cout << "Connected with " << std::to_string(sockAux->getRemotePort()) << std::endl;
					}
					else if (status == sf::Socket::Disconnected) std::cout << "Couldn't connect to " << aDir[i - 1].IP << " --> Disconnected." << std::endl;
					else if (status == sf::Socket::Error) std::cout << "Couldn't connect to " << aDir[i - 1].IP << " --> Error." << std::endl;
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

	while (aPeers.size() != NUM_PLAYERS - 1) {

		sf::TcpSocket* sockAux = new sf::TcpSocket;
		status = listener.accept(*sockAux);

		if (status == sf::Socket::Done) {
			std::cout << "New Connection accepted connection with: " << std::to_string(sockAux->getRemotePort()) << std::endl;
			aPeers.push_back(sockAux);
		}
	}
	listener.close(); //una vegada tinc tots els peers ja puc tancar el listener
	readyToPlay = true;


	if (readyToPlay) {
		bingo = GAME_HASNT_STARTED;

		do {
			NonBlockingChat();
		} while (bingo != GAME_HAS_FINISHED);
	}

	//fer disconnects
	return 0;
}


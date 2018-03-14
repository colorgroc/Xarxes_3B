//TALLER 2 - ANNA PONCE I MARC SEGARRA

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <time.h>
#include <chrono>

#include "Player.cpp"
#include "Game.cpp"


#define MAX_MENSAJES 25
#define RECEIVED 1
#define WRITED 2
#define DUEGAME 4
#define CONNECTION 3
#define NUM_PLAYERS 2
#define PUERTO 50000

enum stateGame { WAIT_FOR_ALL_PLAYERS, ALL_PLAYERS_CONNECTED, GAME_HAS_STARTED, GAME_HAS_FINISHED } bingo;

struct DIRECTIONS {
	std::string IP;
	unsigned short PORT;
};

int numPlayers;
std::vector<sf::TcpSocket*> aPeers;
sf::TcpSocket sock;
std::vector<DIRECTIONS> aDir;
sf::Socket::Status status;
sf::String mensaje;
std::mutex myMutex;

std::vector<std::string> aMensajes;
std::string textoAEnviar = "";
size_t bSent;
sf::String mensajeBook;

bool readyToPlay;
Player *player;
Game *myGame;
unsigned short myPortPlayer;
bool startThreads = false;

inline bool isInteger(const std::string & s)  //https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
{
	if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

	char * p;
	strtol(s.c_str(), &p, 10);

	return (*p == 0);
}

void shared_cout(std::string msg, int option) {

	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

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

				if (command == "BINGO") {
					//mostar que el jugador ha guanyat
					//cambiar estat del bingo a acabat
					aMensajes.push_back(msg);
					bingo = GAME_HAS_FINISHED;
				}
				else if (command == "LINE") {
					//mostar que el jugador ha fet linia
					aMensajes.push_back(msg);
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
		else if (option == DUEGAME) { aMensajes.push_back("Bingo: " + msg); }
	}
}

void SendToOthersPeersDueGame(std::string msg) {
	size_t bSent;
	msg.append("_");
	for (int i = 1; i <= aPeers.size(); i++) { //envio a los demás peers
		status = aPeers[i - 1]->send(msg.c_str(), msg.length(), bSent);
	}
}

void EveryTimeThrowNumber() {

	while (!startThreads) {
		//wait
	}

	while (bingo != GAME_HAS_FINISHED) {
		std::this_thread::sleep_for(std::chrono::seconds(7));
		int temp = myGame->RandomWithoutRepetiton();

		if (temp == -1) { //vol dir que tots els numeros de dintre el bingo ja han estat tirats
			bingo = GAME_HAS_FINISHED;
		}
		if (bingo == GAME_HAS_STARTED) {
			shared_cout(std::to_string(myGame->getCurrentNumberPlaying()), DUEGAME);
		}

	}



}


void NonBlockingChat() {
	while (!startThreads) {
		//wait
	}

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

	//creacio de segona pantalla que mostra sempre la cartilla del jugador
	sf::RenderWindow windowBook;
	sf::Vector2i screenDimensionsBook(300, 300);
	windowBook.create(sf::VideoMode(screenDimensionsBook.x, screenDimensionsBook.y), "MyBook");

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

	//linia separadora
	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(255, 0, 0, 255));
	separator.setPosition(0, 550);

	/////// cartilla
	sf::Text bookText(mensajeBook, font, 14);
	bookText.setFillColor(sf::Color(255, 255, 255));
	bookText.setStyle(sf::Text::Bold);
	///////

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
					//whoHasTalked = aPeers[i]->getRemotePort();
					buffer[bytesReceived] = '\0';
					shared_cout(buffer, RECEIVED);
				}
				else if (status == sf::Socket::Disconnected)
				{
					//elimino el peer q s'ha desconnectat de la llista de peers
					shared_cout("El puerto " + std::to_string(aPeers[i - 1]->getRemotePort()) + " se ha desconectado.", CONNECTION);
					aPeers.erase(aPeers.begin() + (i - 1));
				}
			}
		}


		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				windowBook.close();
				window.close();
				bingo = GAME_HAS_FINISHED;
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape) {
					windowBook.close();
					window.close();
					bingo = GAME_HAS_FINISHED;
				}

				else if (evento.key.code == sf::Keyboard::Return)
				{
					bool toSend = false;
					std::string s_mensaje;
					size_t bSent;

					if (mensaje == "exit") { //puedes salir siempre que quieras
						mensaje = "Disconnected";
					}

					if (bingo != GAME_HAS_FINISHED) {
						////////////////////
						//segons el que escriu el jugador per consola s'envia un command mes el missatge
						if (bingo == GAME_HAS_STARTED) {
							if (mensaje == "line") {
								int temp = player->CheckLine();
								if (temp != 0) {
									s_mensaje = ("LINE_The player " + std::to_string(player->getPlayerInfo()) + " has " + std::to_string(temp) + " lines_");
									shared_cout("Number of Lines " + std::to_string(temp), DUEGAME); //ho notifico al jugador
									toSend = true;
								}
								else {
									shared_cout("You dont have any lines yet", DUEGAME); //ho notifico al jugador
								}
							}
							else if (mensaje == "bingo") {
								player->CheckBingo();
								if (player->getBingo()) {
									s_mensaje = ("BINGO_Congratulations! The player " + std::to_string(player->getPlayerInfo()) + " is the Winner!_");
									shared_cout("Congratulations! I am the Winner!", DUEGAME); //ho notifico al jugador
									shared_cout("You have won " + std::to_string(myGame->getPot()) + "!", DUEGAME); //ho notifico al jugador
									player->setMoney(myGame->getPot());
									toSend = true;
									bingo = GAME_HAS_FINISHED;
								}
								else {
									shared_cout("You dont have bingo yet!", DUEGAME); //ho notifico al jugador
								}
							}
							else if (isInteger(mensaje)) {
								std::string temp = mensaje;
								if (player->CheckNumber(std::stoi(temp), myGame->getCurrentNumberPlaying()))
								{
									shared_cout("The number is correct!", DUEGAME); //ho notifico al jugador
									mensajeBook.clear();
									mensajeBook = player->bookReadyToString();
								}
								else {
									shared_cout("The number is incorrect!", DUEGAME); //ho notifico al jugador
								}
							}
							else if (mensaje != "Disconnected") {
								s_mensaje = "MESSAGE_The player " + std::to_string(player->getPlayerInfo()) + " says: ";
								s_mensaje.append(mensaje);
								s_mensaje.append("_");
								toSend = true;

							}
						}
						else if (bingo == ALL_PLAYERS_CONNECTED && mensaje != "Disconnected") {
							s_mensaje = "MESSAGE_The player " + std::to_string(player->getPlayerInfo()) + " says: ";
							s_mensaje.append(mensaje);
							s_mensaje.append("_");
							toSend = true;
						}

						if (toSend) {
							for (int i = 1; i <= aPeers.size(); i++) { //envio a los demás peers

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
							if (status == sf::Socket::Done)
								shared_cout(mensaje, WRITED);
						}


						if (mensaje == "Disconnected") {
							bingo = GAME_HAS_FINISHED;
							window.close();
							windowBook.close();
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

		//// cartilla
		bookText.setString(mensajeBook);
		windowBook.draw(bookText);
		windowBook.display();
		windowBook.clear();
	}
}

bool ConnectWithAllPeers() {
	//el peer es conecta amb el bootstrap
	status = sock.connect("localhost", PUERTO);

	if (status != sf::Socket::Done) {
		std::cout << "Error de connexion";
	}
	else {
		std::cout << "Connected" << std::endl;
		//std::cout << "I'm Port " << std::to_string(sock.getLocalPort()) << std::endl;
		sf::Packet packet;
		status = sock.receive(packet); //rebo la info de tots els altres peers
		if (status == sf::Socket::Done) {
			//rebo la quantitat de peers que hi ha connectats
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

						std::cout << "Connected with Port " << std::to_string(sockAux->getRemotePort()) << "." << std::endl;
					}
					else if (status == sf::Socket::Disconnected) std::cout << "Couldn't connect to " << aDir[i - 1].IP << " --> Disconnected." << std::endl;
					else if (status == sf::Socket::Error) std::cout << "Couldn't connect to " << aDir[i - 1].IP << " --> Error." << std::endl;
				}
			}
			else {
				std::cout << "There are no peers yet." << std::endl;
			}
		}
	}

	//vaig escolatat els nous peers que envia el bootstrap i el peer shi contecta
	myPortPlayer = sock.getLocalPort(); //guardo el port local
	sock.disconnect(); //desconecto el socket amb el bootstrap
	sf::TcpListener listener;
	listener.listen(myPortPlayer); //escolto

	while (aPeers.size() != NUM_PLAYERS - 1) {

		sf::TcpSocket* sockAux = new sf::TcpSocket;
		status = listener.accept(*sockAux);

		if (status == sf::Socket::Done) {
			std::cout << "New Connection accepted with Port: " << std::to_string(sockAux->getRemotePort()) << std::endl;
			aPeers.push_back(sockAux);
		}
	}
	listener.close(); //una vegada tinc tots els peers ja puc tancar el listener
	readyToPlay = true;

	return readyToPlay;
}


int main()
{
	bingo = WAIT_FOR_ALL_PLAYERS;
	std::thread t1(&EveryTimeThrowNumber);
	std::thread t2(&NonBlockingChat);

	if (bingo == WAIT_FOR_ALL_PLAYERS) {

		if (ConnectWithAllPeers()) {
			bingo = ALL_PLAYERS_CONNECTED;
		}
	}
	if (bingo == ALL_PLAYERS_CONNECTED) {
		shared_cout("The Game has started!", DUEGAME); //ho notifico al jugador
		myGame = new Game();
		player = new Player(myPortPlayer);
		myGame->CalculatePot(*player, NUM_PLAYERS);
		shared_cout("The money inside the pot: " + std::to_string(myGame->getPot()), DUEGAME); //ho notifico al jugador
		mensajeBook = player->bookReadyToString();
		startThreads = true;
		bingo = GAME_HAS_STARTED;
	}

	if (bingo == GAME_HAS_STARTED) {
		do {
			if (aPeers.empty()) {
				bingo = GAME_HAS_FINISHED;
			}

		} while (bingo != GAME_HAS_FINISHED);
	}


	if (bingo == GAME_HAS_FINISHED) {
		if (!aPeers.empty()) {

			SendToOthersPeersDueGame("GAMEFINISHED_"); //si hi han jugadors els hi dic
		}
		shared_cout("Game Finshed", DUEGAME); //ho notifico al jugador

		aPeers.clear();
	}

	//fer disconnects
	t1.join();
	t2.join();
	system("exit");
	return 0;
}


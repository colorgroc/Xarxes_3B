//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <GlobalValues.h>

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;

sf::UdpSocket socket;
sf::Socket::Status status;
int32_t packetID = 1;
sf::Clock c;
int32_t IDPartidaJoin = 0;
Jugador * myPlayer;
std::map <int32_t, InterpolationAndStuff> opponents;
sf::Clock clockPositions;
int32_t idMovements = 1;
int32_t idUltimMoviment = 0;
Walls * myWalls;
int32_t winner;
sf::RenderWindow window;

int16_t positionY = 100;
bool alreadySaidWinner, GTFO, once, create, join, name, password, maxNum, exitGame, writePassword, disconnected, sortByName, sortByNameDown, sortByConnected, sortByConnectedDown, sortByMax, sortByMaxDown, login, sign, username, passwordConnection, mail, connected, globalChat;
std::map <int32_t, PartidaClient> partidas;

std::vector<PartidaClient> vectorListaPartidas;
std::vector<sf::RectangleShape> listButtons;

sf::Color grey = sf::Color(169, 169, 169);
sf::Color greyFosc = sf::Color(49, 51, 53);

std::string textoAEnviar = "";
std::vector<std::string> aMensajes;
sf::String mensaje;

std::mutex myMutex;

void shared_cout(std::string msg, bool received) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora
	
	if (msg != "") {
		if (globalChat) {
			if (received) aMensajes.push_back("Global - Mensaje recibido: " + msg); 
			else aMensajes.push_back(msg); 
		}
		else {
			if (received) aMensajes.push_back("Game - Mensaje recibido: " + msg);
			else aMensajes.push_back(msg);
		}
	}
}



void Chat() {
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
						std::string s_mensaje;

						s_mensaje = mensaje;
						
						sf::Packet p;
						if (globalChat) //ferho per packets
							p << GLOBAL_CHAT << s_mensaje;
						else p << GAME_CHAT << s_mensaje;

						status = socket.send(p, serverIP, serverPORT);
					
						shared_cout(mensaje, false);

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error)
								shared_cout("Ha fallado el envio", false);
							if (status == sf::Socket::Disconnected)
								shared_cout("Disconnected", false);

						}
					}
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					mensaje = "";
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

void Resend() {

	for (std::map<int32_t, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
		status = socket.send(msg->second, "localhost", PORT);
		int32_t cmd;
		msg->second >> cmd;
		//std::cout << "Enviado " << cmd << std::endl;
		if (cmd == SIGNUP)
			std::cout << "Enviado SIGNUP" << std::endl;
		if (cmd == NEW_GAME)
			std::cout << "Enviado NEW_GAME" << std::endl;
		if (status == sf::Socket::Error) {
			std::string cmd;
			msg->second >> cmd;
			std::cout << "Error sending the message. Client to Server." << "Message IP: " << std::to_string(msg->first) << "Message: " << cmd << std::endl;
		}
		else if (status == sf::Socket::Disconnected) {
			std::cout << "Error sending the message. Server disconnected." << std::endl;
			//connected = false;
			socket.unbind();
			system("exit");
		}
	}
}


void RefreshPartidas(int32_t id, std::string name, int32_t conn, int32_t max) {

	sf::RectangleShape button(sf::Vector2f(1150, 50.f));
	button.setPosition(50, positionY);
	button.setFillColor(sf::Color::Transparent);

	vectorListaPartidas.push_back(PartidaClient{ id, name, conn, max });
	listButtons.push_back(button);
	positionY += 50;

	if(sortByName)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameUp);
	else if (sortByNameDown)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameDown);
	else if(sortByConnectedDown)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionDown);
	else if (sortByConnected)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionUp);
	else if (sortByMaxDown)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumDown);
	else if (sortByMax)
		std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumUp);
	else std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameUp);
}

void DeletePartidas(int32_t gID) {
	
	for (int8_t i = 0; i < partidas.size(); i++) {
		if (vectorListaPartidas[i].id == gID) vectorListaPartidas.erase(vectorListaPartidas.begin(), vectorListaPartidas.begin() + i);
	}
	listButtons.pop_back();
	partidas.erase(gID);
}



void SendACK(int cmd, int32_t pID) {
	sf::Packet packet;
	std::string com;

	if (cmd == ACK_DISCONNECTION) {
		com = "DISCONNECTION";
		packet << cmd << pID << myPlayer->ID << myPlayer->IDPartida;
	}
	else if (cmd == ACK_NEW_CONNECTION) {
		com = "NEW_CONNECTION";
		packet << cmd << pID << myPlayer->ID << myPlayer->IDPartida;
	}
	else if (cmd == ACK_PING) {
		com = "ACK_PING";
		packet << cmd << pID << myPlayer->ID << myPlayer->IDPartida;
	}
	else if (cmd == ACK_PING_LOBBY) {
		com = "ACK_PING_LOBBY";
		packet << cmd << pID << myPlayer->ID;
	}
	else if (cmd == ACK_QUI_LA_PILLA) {
		packet << cmd << pID << myPlayer->ID << myPlayer->IDPartida;
		com = "ACK_QUI_LA_PILLA";
	}
	else if (cmd == ACK_WINNER) {
		packet << cmd << pID << myPlayer->ID << myPlayer->IDPartida;
		com = "ACK_WINNER";
	}

	
	status = socket.send(packet, "localhost", PORT);
	if (status == sf::Socket::Error) std::cout << "Error. " << com << std::endl;
	else if (status == sf::Socket::Disconnected) {
		std::cout << "Server disconnected. " << com << std::endl;
		opponents.clear();
		socket.unbind();
		system("exit");
	}
	packet.clear();
}

void ReceiveData() {
	//nonblocking
	sf::Packet packet;
	int cmd = 0;
	int32_t opponentId = 0;
	std::string opponentNickname = "";
	int32_t packetIDRecived = 0;

	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		float rndPacketLoss = GetRandomFloat();
		if (rndPacketLoss < PERCENT_PACKETLOSS) {
			std::cout << "Paquet perdut" << std::endl;
		}
		else {
			packet >> cmd >> packetIDRecived;

			if (cmd == PING) {
				SendACK(ACK_PING, packetIDRecived);
			}
			if (cmd == PING_LOBBY) {
				SendACK(ACK_PING_LOBBY, packetIDRecived);
			}
			if (cmd == ID_ALREADY_CONNECTED) {
				//std::cout << "This ID it's already connected." << std::endl;
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
				//login = sign = false;
				//connected = false;
				//ConnectionWithServer();
				GTFO = true;
			}
			//else if (cmd == ID_ALREADY_EXISTS) {

			//}
			else if (cmd == ID_ALREADY_PLAYING) {
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
				std::cout << "This user is already playing a game" << std::endl;
				join = true;
				writePassword = false;
			}
			else if (cmd == PASSWORD_INCORRECT) {
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);			
				}
				std::cout << "Incorrect Password" << std::endl;
			}
			else if (cmd == ACK_LOGIN) {
				int32_t numPartidas = 0;
				packet >> myPlayer->ID >> numPartidas;
				for (int8_t i = 0; i < numPartidas; i++) {
					int32_t gID = 0;
					int32_t maxP = 0;
					int32_t conn = 0;
					std::string name = "";
					packet << gID << name << conn << maxP;
					partidas.insert(std::make_pair(gID, PartidaClient{ gID, name, maxP, conn }));
				}
				
				//text en vermell error username i error password
				std::cout << "Connection with server." << std::endl;
				//connected = true;
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
			}
			else if (cmd == ACK_SIGNUP) {
				int32_t numPartidas = 0;
				packet >> myPlayer->ID >> numPartidas;

				for (int8_t i = 0; i < numPartidas; i++) {
					int32_t gID = 0;
					int32_t maxP = 0;
					int32_t conn = 0;
					std::string name = "";
					packet << gID << name << conn << maxP;
					partidas.insert(std::make_pair(gID, PartidaClient{ gID, name, maxP, conn }));
				}
				//text en vermell error username, error mail i error password
				std::cout << "Connection with server." << std::endl;
				//connected = true;
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
			}
			else if (cmd == NEW_GAME_CREATED) {
				int32_t gID = 0;
				std::string gName = "";
				int32_t gMaxP = 0;
				packet >> gID >> gName >> gMaxP;
				partidas.insert(std::make_pair(gID, PartidaClient{ gID,gName, 1, gMaxP }));
				//ENVIAR ACK
				RefreshPartidas(gID, gName, 1, gMaxP);
			}
			else if (cmd == GAME_DELETED) {
				int32_t gID = 0;
				packet >> gID;
				DeletePartidas(gID);
			}
			else if (cmd == GAMESTARTED) {
				//SendACK(ACK_GAMESTARTED, packetIDRecived);
				std::cout << "The game PILLAPILLA started." << std::endl;
			}
			else if (cmd == WELCOME) {
				//if (myPlayer->ID == 0) {
					int32_t numOfOpponents = 0;
					packet >> myPlayer->IDPartida >> myPlayer->position >> numOfOpponents;
					std::cout << "ID Partida: " << myPlayer->IDPartida << " Pos: " << myPlayer->position.x << ", " << myPlayer->position.y << " Num Opo: " << numOfOpponents;
					if (numOfOpponents > 0) {
						//treiem del packet la ID i la pos de cada oponent
						for (int i = 0; i < numOfOpponents; i++) {
							//int8_t opponentId;
							Position oPos;
							packet >> opponentId >> opponentNickname >> oPos;
							std::cout << "ID Opo: " << opponentId << "Nickname: "  << opponentNickname << " Pos: " << myPlayer->position.x << ", " << myPlayer->position.y;
							opponents.insert(std::make_pair(opponentId, InterpolationAndStuff{ oPos, oPos, false, opponentNickname }));
						}
					}
					if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
						myPlayer->resending.erase(packetIDRecived);
					}
					std::cout << "WELCOME! " << " Client ID: " << std::to_string(myPlayer->ID) << " Initial Position: " << std::to_string(myPlayer->position.x) << ", " << std::to_string(myPlayer->position.y) << std::endl;
				//}
			}

			else if (cmd == NEW_CONNECTION) {
				packet >> opponentId >> opponentNickname;
				if (opponents.find(opponentId) == opponents.end()) {
					Position pos;
					packet >> pos;
					std::cout << "A new opponent connected. ID: " << std::to_string(opponentId) << " Nickname: " << opponentNickname <<" Position: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << std::endl;
					opponents.insert(std::make_pair(opponentId, InterpolationAndStuff{ pos, pos, false, opponentNickname }));
				}
				SendACK(ACK_NEW_CONNECTION, packetIDRecived);
			}
			else if (cmd == OK_POSITION) {

				//solucionar problemes d'ordre de paquets
				//si hi han id mes petits a la llista del client es poden borrar ja que es va a la poscio més allunyada
				//si la posicio reb un -1 -1 vol dir que la poscio es incorrecta i per tant no ens movem i borrem les altres poscions anterirors acumulades
				int32_t idMov;
				Position tempPos;
				packet >> idMov >> tempPos;

				if (tempPos.x == -1 && tempPos.y == -1) {
					Position lastPos;
					packet >> lastPos;
					//es invalida la posicio i per tant no actualitzem posicio, borrem de la llista accum
					std::cout << "Out"; //els jugadors a vegades poden sortir ja al limit (falta arreglar, mes endavant) i no es poden moure!!!
					if (myPlayer->MapAccumMovements.find(idMov) != myPlayer->MapAccumMovements.end()) {
						myPlayer->MapAccumMovements.erase(idMov);
					}
					myPlayer->position = lastPos;
				}
				else {
					//valida

					if (idMov > idUltimMoviment) { //nomes si es mes gran valido , eliminem de la llista de accum
						idUltimMoviment = idMov;
						//notMove = false;
						//myPlayer->position = tempPos;
					}
					else { //ja s'ha acceptat un moviment posterior i per tant aquest moviment anterior no s'executa i es dona per valid, eliminem de la llista de accum
						if (myPlayer->MapAccumMovements.find(idMov) != myPlayer->MapAccumMovements.end()) {
							myPlayer->MapAccumMovements.erase(idMov);
							//notMove = true;
						}
					}

				}
			}
			else if (cmd == REFRESH_POSITIONS) {
				packet >> opponentId;
				if (opponents.find(opponentId) != opponents.end()) {
					Position pos;
					packet >> pos;
					opponents.find(opponentId)->second.newPos = pos;

					//si les posicions son diferents calculem passos interpolacio
					if (opponents.find(opponentId)->second.newPos.x != opponents.find(opponentId)->second.lastPos.x) {
						if (opponents.find(opponentId)->second.lastPos.x > opponents.find(opponentId)->second.newPos.x) { //direccio moviment esquerra
							for (int16_t i = opponents.find(opponentId)->second.lastPos.x; i >= opponents.find(opponentId)->second.newPos.x; i--) {
								opponents.find(opponentId)->second.middlePositions.push(Position{ i, opponents.find(opponentId)->second.lastPos.y });
							}
						}
						else { //direccio moviment dreta
							for (int16_t i = opponents.find(opponentId)->second.lastPos.x; i <= opponents.find(opponentId)->second.newPos.x; i++) {
								opponents.find(opponentId)->second.middlePositions.push(Position{ i, opponents.find(opponentId)->second.lastPos.y });
							}
						}
					}
					if (opponents.find(opponentId)->second.newPos.y != opponents.find(opponentId)->second.lastPos.y) {
						if (opponents.find(opponentId)->second.lastPos.y > opponents.find(opponentId)->second.newPos.y) { //direccio moviment dalt
							for (int16_t i = opponents.find(opponentId)->second.lastPos.y; i >= opponents.find(opponentId)->second.newPos.y; i--) {
								opponents.find(opponentId)->second.middlePositions.push(Position{ opponents.find(opponentId)->second.lastPos.x, i });
							}
						}
						else { //direccio moviment baix
							for (int16_t i = opponents.find(opponentId)->second.lastPos.y; i <= opponents.find(opponentId)->second.newPos.y; i++) {
								opponents.find(opponentId)->second.middlePositions.push(Position{ opponents.find(opponentId)->second.lastPos.x, i });
							}
						}
					}
					opponents.find(opponentId)->second.lastPos = opponents.find(opponentId)->second.newPos; //ja s'han calculat i guardat passos intermitjos, per tant actualitzem lastPos
																											//si no s'ha mogut no cal fer res
				}

			}
			else if (cmd == DISCONNECTION) {
				packet >> opponentId;
				if (opponents.find(opponentId) != opponents.end()) {
					std::cout << "An opponent disconnected. ID: " << std::to_string(opponentId) << std::endl;
					opponents.erase(opponentId);
				}
				SendACK(ACK_DISCONNECTION, packetIDRecived);
			}
			else if (cmd == QUI_LA_PILLA) {
				packet >> opponentId; //no te pq ser la del oponent
				if (opponentId == myPlayer->ID) {
					if (!myPlayer->laParo) {
						myPlayer->laParo = true;
						std::cout << "La paro jo." << std::endl;
					}
				}
				else {
					if (opponents.find(opponentId) != opponents.end()) {

						if (!opponents[opponentId].laPara) {
							opponents[opponentId].laPara = true;
							std::cout << "La para l'oponent amb ID: " << std::to_string(opponentId) << std::endl;
						}
					}
				}
				SendACK(ACK_QUI_LA_PILLA, packetIDRecived);
			}
			else if (cmd == WINNER) {
				packet >> winner; //no te pq ser la del oponent
				SendACK(ACK_WINNER, packetIDRecived);
				if (!alreadySaidWinner) {
					if (winner == myPlayer->ID) {
						std::cout << "------ I'M THE WINNER! ------" << std::endl;
						std::cout << "------ GAME FINISHED ------" << std::endl;
					}
					else {
						std::cout << "------ OPPONENT " << winner << " IS THE WINNER! ------" << std::endl;
						std::cout << "------ GAME FINISHED ------" << std::endl;
					}
					alreadySaidWinner = true;
				}
			}
		}

	}

	packet.clear();
}

void Lobby() {
	window.create(sf::VideoMode::getDesktopMode(), "Lobby", sf::Style::Fullscreen);
	window.clear();
	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
		std::cout << "Can't find the font file" << std::endl;

	//-------------- create ----------------------------
	sf::RectangleShape createButton(sf::Vector2f(300, 50.f));
	createButton.setPosition(window.getSize().x / 2 / 2, 200);
	createButton.setFillColor(sf::Color::White);

	sf::Text createText;
	createText.setFont(font);
	createText.setStyle(sf::Text::Regular);
	createText.setString("Create Game");
	createText.setFillColor(sf::Color::Black);
	createText.setCharacterSize(48);
	createText.setPosition(window.getSize().x / 2 / 2 + 20, createButton.getPosition().y - 10);

	//-------------- join ----------------------------
	sf::RectangleShape joinButton(sf::Vector2f(300, 50.f));
	joinButton.setPosition(window.getSize().x / 2 / 2, 300);
	joinButton.setFillColor(sf::Color::White);

	sf::Text joinText;
	joinText.setFont(font);
	joinText.setStyle(sf::Text::Regular);
	joinText.setString("Join Game");
	joinText.setFillColor(sf::Color::Black);
	joinText.setCharacterSize(48);
	joinText.setPosition(window.getSize().x / 2 / 2 + 20, joinButton.getPosition().y - 10);

	//-------------- exit ----------------------------
	sf::RectangleShape exitButton(sf::Vector2f(200.f, 50.f));
	exitButton.setPosition(window.getSize().x / 2 / 2, 400);
	exitButton.setFillColor(sf::Color::Red);

	sf::Text exitText;
	exitText.setFont(font);
	exitText.setStyle(sf::Text::Regular);
	exitText.setString("Exit");
	exitText.setFillColor(sf::Color::White);
	exitText.setCharacterSize(48);
	exitText.setPosition(window.getSize().x / 2 / 2 + 50, exitButton.getPosition().y - 10);

	//-------------- back ----------------------------
	sf::RectangleShape backButton(sf::Vector2f(200.f, 50.f));
	backButton.setPosition(50, window.getSize().y - 100);
	backButton.setFillColor(sf::Color::Yellow);

	sf::Text backText;
	backText.setFont(font);
	backText.setStyle(sf::Text::Regular);
	backText.setString("Back");
	backText.setFillColor(sf::Color::Black);
	backText.setCharacterSize(48);
	backText.setPosition(50 + 50, backButton.getPosition().y - 10);

	//-------------- ok ----------------------------
	sf::RectangleShape okButton(sf::Vector2f(200.f, 50.f));
	okButton.setPosition(window.getSize().x - 300, window.getSize().y - 100);
	okButton.setFillColor(sf::Color::Green);

	sf::Text okText;
	okText.setFont(font);
	okText.setStyle(sf::Text::Regular);
	okText.setFillColor(sf::Color::Black);
	okText.setCharacterSize(48);
	okText.setPosition(window.getSize().x - 300 + 50, okButton.getPosition().y - 10);


	//-------------- Create: Name ----------------------------
	sf::Text createNameText;
	createNameText.setFont(font);
	createNameText.setStyle(sf::Text::Regular);
	createNameText.setString("Name: ");
	createNameText.setFillColor(sf::Color::White);
	createNameText.setCharacterSize(48);
	createNameText.setPosition(window.getSize().x / 2 / 2, 150);

	sf::RectangleShape createNameButton(sf::Vector2f(200, 50.f));
	createNameButton.setPosition(window.getSize().x / 2 / 2, 200);
	createNameButton.setFillColor(sf::Color::White);

	sf::String nameInput;
	sf::Text nameText("", font, 48);
	nameText.setPosition(window.getSize().x / 2 / 2, 200 - 10);
	nameText.setFillColor(sf::Color::Black);

	//-------------- Create: Password ----------------------------
	sf::Text createPassText;
	createPassText.setFont(font);
	createPassText.setStyle(sf::Text::Regular);
	createPassText.setString("Password:");
	createPassText.setFillColor(sf::Color::White);
	createPassText.setCharacterSize(48);
	createPassText.setPosition(window.getSize().x / 2 / 2, 250);

	sf::RectangleShape createPassButton(sf::Vector2f(200, 50.f));
	createPassButton.setPosition(window.getSize().x / 2 / 2, 300);
	createPassButton.setFillColor(sf::Color::White);

	sf::String passInput;
	sf::Text passText("", font, 48);
	passText.setPosition(window.getSize().x / 2 / 2, 300 - 10);
	passText.setFillColor(sf::Color::Black);

	//sf::String checkPassInput;
	//sf::Text checkPassText("", font, 48);
	//passText.setPosition(window.getSize().x / 2 / 2, 300 - 10);
	//passText.setFillColor(sf::Color::Black);

	//-------------- Create: MaxPlayers ----------------------------
	sf::Text createMaxText;
	createMaxText.setFont(font);
	createMaxText.setStyle(sf::Text::Regular);
	createMaxText.setString("Max Players:");
	createMaxText.setFillColor(sf::Color::White);
	createMaxText.setCharacterSize(48);
	createMaxText.setPosition(window.getSize().x / 2 / 2, 350);

	sf::RectangleShape createMaxButton(sf::Vector2f(100, 50.f));
	createMaxButton.setPosition(window.getSize().x / 2 / 2, 400);
	createMaxButton.setFillColor(sf::Color::White);

	sf::String numInput;
	sf::Text numText("", font, 48);
	numText.setPosition(window.getSize().x / 2 / 2, 400 - 10);
	numText.setFillColor(sf::Color::Black);

	//-------------- Join: List ----------------------------

	sf::Text nameTextButton;
	nameTextButton.setFont(font);
	nameTextButton.setStyle(sf::Text::Regular);
	nameTextButton.setFillColor(sf::Color::White);
	nameTextButton.setCharacterSize(48);

	sf::Text connTextButton;
	connTextButton.setFont(font);
	connTextButton.setStyle(sf::Text::Regular);
	connTextButton.setFillColor(sf::Color::White);
	connTextButton.setCharacterSize(48);

	sf::Text maxTextButton;
	maxTextButton.setFont(font);
	maxTextButton.setStyle(sf::Text::Regular);
	maxTextButton.setFillColor(sf::Color::White);
	maxTextButton.setCharacterSize(48);


	for (std::map<int32_t, PartidaClient>::iterator it = partidas.begin(); it != partidas.end(); ++it) {

		sf::RectangleShape button(sf::Vector2f(1150, 50.f));
		button.setPosition(50, positionY);
		button.setFillColor(sf::Color::Transparent);

		vectorListaPartidas.push_back(PartidaClient{ it->second.id, it->second.name, it->second.numPlayersConnected, it->second.maxPlayers });
		listButtons.push_back(button);
		positionY += 50;
	}

	//------------- Join: Capçalera List -------------------
	sf::Text capName;
	capName.setFont(font);
	capName.setStyle(sf::Text::Italic);
	capName.setString("Name");
	capName.setFillColor(sf::Color::White);
	capName.setCharacterSize(26);
	capName.setPosition(100, 50);

	sf::Text capCon;
	capCon.setFont(font);
	capCon.setStyle(sf::Text::Italic);
	capCon.setString("Connected");
	capCon.setFillColor(sf::Color::White);
	capCon.setCharacterSize(26);
	capCon.setPosition(900, 50);

	sf::Text capMax;
	capMax.setFont(font);
	capMax.setStyle(sf::Text::Italic);
	capMax.setString("Max Players");
	capMax.setFillColor(sf::Color::White);
	capMax.setCharacterSize(26);
	capMax.setPosition(capCon.getPosition().x + capCon.getLocalBounds().width + 50, 50);

	//-------------- Join: Sort By Name ----------------------------
	sf::RectangleShape sortNameButton(sf::Vector2f(150, 30.f));
	sortNameButton.setPosition(window.getSize().x - 600, 50);
	sortNameButton.setFillColor(sf::Color::Blue);

	sf::Text sortNameText;
	sortNameText.setFont(font);
	sortNameText.setStyle(sf::Text::Italic);
	sortNameText.setString("by Name");
	sortNameText.setFillColor(sf::Color::White);
	sortNameText.setCharacterSize(26);
	sortNameText.setPosition(sortNameButton.getPosition().x + 20, sortNameButton.getPosition().y - 5);

	//-------------- Join: Sort By Connected ----------------------------
	sf::RectangleShape sortConButton(sf::Vector2f(150, 30.f));
	sortConButton.setPosition(window.getSize().x - 440, 50);
	sortConButton.setFillColor(grey);

	sf::Text sortConText;
	sortConText.setFont(font);
	sortConText.setStyle(sf::Text::Italic);
	sortConText.setString("by nº Con");
	sortConText.setFillColor(sf::Color::White);
	sortConText.setCharacterSize(26);
	sortConText.setPosition(sortConButton.getPosition().x + 20, sortConButton.getPosition().y - 5);

	//-------------- Join: Sort By Max Players ----------------------------
	sf::RectangleShape sortMaxButton(sf::Vector2f(150, 30.f));
	sortMaxButton.setPosition(window.getSize().x - 280, 50);
	sortMaxButton.setFillColor(grey);

	sf::Text sortMaxText;
	sortMaxText.setFont(font);
	sortMaxText.setStyle(sf::Text::Italic);
	sortMaxText.setString("by nº Max");
	sortMaxText.setFillColor(sf::Color::White);
	sortMaxText.setCharacterSize(26);
	sortMaxText.setPosition(sortMaxButton.getPosition().x + 20, sortMaxButton.getPosition().y - 5);

	while (window.isOpen())
	{
		sf::Event Event;
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
		ReceiveData();

		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}
		while (window.pollEvent(Event))
		{
			switch (Event.type)
			{
			case sf::Event::Closed:
				window.close();
				exitGame = disconnected = true;
				break;
			case sf::Event::MouseMoved:
			{
				//Create
				if (createButton.getGlobalBounds().contains(mousePosF))
					createButton.setFillColor(grey);
				else createButton.setFillColor(sf::Color::White);
				//join
				if (joinButton.getGlobalBounds().contains(mousePosF))
					joinButton.setFillColor(grey);
				else joinButton.setFillColor(sf::Color::White);
				//exit
				if (exitButton.getGlobalBounds().contains(mousePosF))
					exitButton.setFillColor(grey);
				else exitButton.setFillColor(sf::Color::Red);
				//back
				if (backButton.getGlobalBounds().contains(mousePosF))
					backButton.setFillColor(grey);
				else backButton.setFillColor(sf::Color::Yellow);
				//ok
				if (okButton.getGlobalBounds().contains(mousePosF))
					okButton.setFillColor(grey);
				else okButton.setFillColor(sf::Color::Green);
				//sortName
				if (sortNameButton.getGlobalBounds().contains(mousePosF))
					sortNameButton.setFillColor(sf::Color::Blue);
				else sortNameButton.setFillColor(grey);
				//sortConnected
				if (sortConButton.getGlobalBounds().contains(mousePosF))
					sortConButton.setFillColor(sf::Color::Blue);
				else sortConButton.setFillColor(grey);
				//sortMaxPlayers
				if (sortMaxButton.getGlobalBounds().contains(mousePosF))
					sortMaxButton.setFillColor(sf::Color::Blue);
				else sortMaxButton.setFillColor(grey);

				if (join) {
					for (int8_t i = 0; i < listButtons.size(); i++) {
						if (listButtons[i].getGlobalBounds().contains(mousePosF))
							listButtons[i].setFillColor(grey);
						else listButtons[i].setFillColor(sf::Color::Transparent);
					}

				}
			}
			break;
			case sf::Event::MouseButtonPressed:
			{
				if (createButton.getGlobalBounds().contains(mousePosF) && !join && !create)
				{
					create = true;
					name = true;
					createNameButton.setFillColor(grey);
					okText.setString("Create");
					break;
				}
				else if (joinButton.getGlobalBounds().contains(mousePosF) && !join && !create)
				{
					join = true;
					sortByName = sortByNameDown = true;
					std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameDown);
					okText.setString("Join");
					break;
				}
				else if (backButton.getGlobalBounds().contains(mousePosF))
				{
					create = join = name = password = maxNum = writePassword = false;
					nameInput.clear();
					numInput.clear();
					passInput.clear();
					nameText.setString("");
					numText.setString("");
					passText.setString("");
					break;
				}
				else if (okButton.getGlobalBounds().contains(mousePosF))
				{
					if (create) {
						sf::Packet packet;
						//std::string str = passText.getString();
						packet << NEW_GAME << packetID << myPlayer->ID << nameInput << passInput << numInput;
						myPlayer->resending.insert(std::make_pair(myPlayer->ID, packet));
						packetID++;
					}
					else if (join) {
						sf::Packet packet;
						//std::string str = passText.getString();
						packet << JOIN_GAME << packetID << myPlayer->ID << IDPartidaJoin << passInput;
						myPlayer->resending.insert(std::make_pair(IDPartidaJoin, packet));
						packetID++;
					}
					
					window.close();
					create = join = name = password = maxNum = writePassword = false;
					nameInput.clear();
					numInput.clear();
					passInput.clear();
					nameText.setString("");
					numText.setString("");
					passText.setString("");
					break;
				}
				else if (exitButton.getGlobalBounds().contains(mousePosF) && !create && !join)
				{
					window.close();
					exitGame = disconnected = true;
					break;
				}
				else if (createNameButton.getGlobalBounds().contains(mousePosF) && create)
				{
					name = true;
					password = maxNum = false;
					break;
				}
				else if (createPassButton.getGlobalBounds().contains(mousePosF) && create)
				{
					password = true;
					name = maxNum = false;
					break;
				}
				else if (createMaxButton.getGlobalBounds().contains(mousePosF) && create)
				{
					maxNum = true;
					name = password = false;
					break;
				}
				else if (sortNameButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByName = true;
					sortByMax = sortByConnected = false;
					if (sortByNameDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameDown);
						sortByNameDown = !sortByNameDown;
					}
					else if (!sortByNameDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameUp);
						sortByNameDown = !sortByNameDown;
					}
					break;
				}
				else if (sortConButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByConnected = true;
					sortByMax = sortByName = false;
					if (sortByConnectedDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionDown);
						sortByConnectedDown = !sortByConnectedDown;
					}
					else if (!sortByConnectedDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionUp);
						sortByConnectedDown = !sortByConnectedDown;
					}
					break;
				}
				else if (sortMaxButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByMax = true;
					sortByName = sortByConnected = false;
					if (sortByMaxDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumDown);
						sortByMaxDown = !sortByMaxDown;
					}
					else if (!sortByMaxDown) {
						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumUp);
						sortByMaxDown = !sortByMaxDown;
					}
					break;
				}
				if (join) {

					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						if (listButtons[i].getGlobalBounds().contains(mousePosF)) {
							writePassword = true;//ensenyar introduccio password
							IDPartidaJoin = vectorListaPartidas[i].id;//em guardo l'id de la partida
							break;
						}
					}
				}
			}
			break;
			case sf::Event::TextEntered:
			{
				//si clico el backspace/borrar/retroceso
				if (Event.text.unicode == '\b' && name) {
					if (nameInput.getSize() > 0)
						nameInput.erase(nameInput.getSize() - 1, 1);
					nameText.setString(nameInput);
				}
				else if (Event.text.unicode == '\b' && (password || writePassword)) {
					if (passInput.getSize() > 0)
						passInput.erase(passInput.getSize() - 1, 1);
					passText.setString(passInput);
				}
				else if (Event.text.unicode == '\b' && maxNum) {
					if (numInput.getSize() > 0)
						numInput.erase(numInput.getSize() - 1, 1);
					numText.setString(numInput);
				}
				//si clico algo q no sigui borrar
				else if (Event.text.unicode < 128 && name)
				{
					nameInput += Event.text.unicode;
					nameText.setString(nameInput);
				}
				else if (Event.text.unicode < 128 && (password || writePassword)) {
					passInput += Event.text.unicode;
					passText.setString(passInput);
				}
				else if (Event.text.unicode < 128 && maxNum) {
					numInput += Event.text.unicode;
					numText.setString(numInput);
				}
			}
			break;

			case sf::Event::KeyPressed:
			{
				if (Event.key.code == sf::Keyboard::Return) {
					if (name) {
						name = false;
						password = true;
						//enviar nameText.getString();
					}
					else if (password) {
						password = false;
						maxNum = true;
						//enviar passText.getString();
					}
					else if (maxNum) {
						maxNum = false;
						//enviar numText.getString();
						break;
					}
					//else if (writePassword) {
					//	sf::Packet packet;
					//	//std::string str = passText.getString();
					//	packet << packetID << myPlayer->ID << IDPartidaJoin << passText.getString();
					//	myPlayer->resending.insert(std::make_pair(myPlayer->ID, packet));
					//	packetID++;
					//}
					break;
				}
			}
			break;
			}
		}

		if (name) createNameButton.setFillColor(grey);
		else createNameButton.setFillColor(sf::Color::White);
		if (password) createPassButton.setFillColor(grey);
		else createPassButton.setFillColor(sf::Color::White);
		if (maxNum) createMaxButton.setFillColor(grey);
		else createMaxButton.setFillColor(sf::Color::White);

		window.clear();
		if (!create && !join) { //mostro lobby principal
			window.draw(createButton);
			window.draw(createText);
			window.draw(joinButton);
			window.draw(joinText);
			window.draw(exitButton);
			window.draw(exitText);
		}
		else if (create) {	//mostro lobby de crear partida
							//name
			window.draw(createNameText);
			window.draw(createNameButton);
			window.draw(nameText);
			//password
			window.draw(createPassText);
			window.draw(createPassButton);
			window.draw(passText);
			//maxNum
			window.draw(createMaxText);
			window.draw(createMaxButton);
			window.draw(numText);
			//back
			window.draw(backButton);
			window.draw(backText);
			//ok
			window.draw(okButton);
			window.draw(okText);
		}
		else if (join) { //mostro lobby de unirse partida

			if (sortByMax) sortMaxButton.setFillColor(sf::Color::Blue);
			else if (sortByConnected) sortConButton.setFillColor(sf::Color::Blue);
			else if (sortByName) sortNameButton.setFillColor(sf::Color::Blue);
			if (!writePassword) {
				for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
					window.draw(listButtons[i]);

					nameTextButton.setString(vectorListaPartidas[i].name);
					nameTextButton.setPosition(listButtons[i].getPosition().x + 20, listButtons[i].getPosition().y - 10);

					connTextButton.setString(std::to_string(vectorListaPartidas[i].numPlayersConnected));
					connTextButton.setPosition(listButtons[i].getPosition().x + 900, listButtons[i].getPosition().y - 10);

					maxTextButton.setString(std::to_string(vectorListaPartidas[i].maxPlayers));
					maxTextButton.setPosition(connTextButton.getPosition().x + connTextButton.getLocalBounds().width + 150, listButtons[i].getPosition().y - 10);

					window.draw(nameTextButton);
					window.draw(connTextButton);
					window.draw(maxTextButton);
				}
			}
			if (!writePassword) {
				//Capçalera
				window.draw(capName);
				window.draw(capCon);
				window.draw(capMax);
				//sortName
				window.draw(sortNameButton);
				window.draw(sortNameText);
				//sortConnected
				window.draw(sortConButton);
				window.draw(sortConText);
				//sortMaxPlayers
				window.draw(sortMaxButton);
				window.draw(sortMaxText);
			}
			else if (writePassword) {
				//password
				window.draw(createPassText);
				window.draw(createPassButton);
				window.draw(passText);
			}
			//back
			window.draw(backButton);
			window.draw(backText);
			//ok
			window.draw(okButton);
			window.draw(okText);


		}
		window.display();
	}
}

void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";
	window.create(sf::VideoMode(500, 750), "Lobby", sf::Style::Default);

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
		std::cout << "Can't find the font file" << std::endl;
	//-------------- login ----------------------------
	sf::RectangleShape loginButton(sf::Vector2f(300, 50.f));
	loginButton.setPosition(window.getSize().x / 2 / 2, 200);
	loginButton.setFillColor(sf::Color::White);

	sf::Text loginText;
	loginText.setFont(font);
	loginText.setStyle(sf::Text::Regular);
	loginText.setString("Login In");
	loginText.setFillColor(sf::Color::Black);
	loginText.setCharacterSize(48);
	loginText.setPosition(window.getSize().x / 2 / 2 + 20, loginButton.getPosition().y - 10);

	//-------------- sign up ----------------------------
	sf::RectangleShape signButton(sf::Vector2f(300, 50.f));
	signButton.setPosition(window.getSize().x / 2 / 2, loginButton.getPosition().y + 100);
	signButton.setFillColor(sf::Color::White);

	sf::Text signText;
	signText.setFont(font);
	signText.setStyle(sf::Text::Regular);
	signText.setString("Sign Up");
	signText.setFillColor(sf::Color::Black);
	signText.setCharacterSize(48);
	signText.setPosition(window.getSize().x / 2 / 2 + 20, signButton.getPosition().y - 10);
	//------------------------------------------------------

	sf::Text titleText;
	titleText.setFont(font);
	titleText.setString("Pilla Pilla");
	titleText.setStyle(sf::Text::Regular);
	titleText.setFillColor(sf::Color::Yellow);
	titleText.setCharacterSize(48);
	titleText.setPosition(window.getSize().x / 2 / 2 + 20, 50);
	//--------- username ----------------------------------------
	sf::Text userNameText;
	userNameText.setFont(font);
	userNameText.setStyle(sf::Text::Regular);
	userNameText.setString("Username: ");
	userNameText.setFillColor(sf::Color::White);
	userNameText.setCharacterSize(48);
	userNameText.setPosition(window.getSize().x / 2 / 2, titleText.getPosition().y + 50);

	sf::RectangleShape usernameButton(sf::Vector2f(300.f, 60.f));
	usernameButton.setPosition(window.getSize().x / 2 / 2, userNameText.getPosition().y + 50);
	usernameButton.setFillColor(sf::Color::White);

	sf::String userInput;
	sf::Text userText("", font, 48);
	userText.setPosition(window.getSize().x / 2 / 2 + 10, usernameButton.getPosition().y - 10);
	userText.setFillColor(sf::Color::Black);

	//--------- passwordConnection ----------------------------------------
	sf::Text passwordConecText;
	passwordConecText.setFont(font);
	passwordConecText.setStyle(sf::Text::Regular);
	passwordConecText.setString("Password: ");
	passwordConecText.setFillColor(sf::Color::White);
	passwordConecText.setCharacterSize(48);
	passwordConecText.setPosition(window.getSize().x / 2 / 2, usernameButton.getPosition().y + 50);

	sf::RectangleShape passConnectionButton(sf::Vector2f(300.f, 60.f));
	passConnectionButton.setPosition(window.getSize().x / 2 / 2, passwordConecText.getPosition().y + 50);
	passConnectionButton.setFillColor(sf::Color::White);

	sf::String passInput;
	sf::Text passText("", font, 48);
	passText.setPosition(window.getSize().x / 2 / 2 + 10, passConnectionButton.getPosition().y - 10);
	passText.setFillColor(sf::Color::Black);

	//--------- mail ----------------------------------------
	sf::Text mailConectText;
	mailConectText.setFont(font);
	mailConectText.setStyle(sf::Text::Regular);
	mailConectText.setString("Mail: ");
	mailConectText.setFillColor(sf::Color::White);
	mailConectText.setCharacterSize(48);
	mailConectText.setPosition(window.getSize().x / 2 / 2, passConnectionButton.getPosition().y + 50);

	sf::RectangleShape mailButton(sf::Vector2f(300.f, 60.f));
	mailButton.setPosition(window.getSize().x / 2 / 2, mailConectText.getPosition().y + 50);
	mailButton.setFillColor(sf::Color::White);

	sf::String mailInput;
	sf::Text mailText("", font, 48);
	mailText.setPosition(window.getSize().x / 2 / 2 + 10, mailButton.getPosition().y - 10);
	mailText.setFillColor(sf::Color::Black);

	//-------------- back ----------------------------
	sf::RectangleShape backButton(sf::Vector2f(200.f, 50.f));
	backButton.setPosition(50, window.getSize().y - 100);
	backButton.setFillColor(sf::Color::Yellow);

	sf::Text backText;
	backText.setFont(font);
	backText.setStyle(sf::Text::Regular);
	backText.setString("Back");
	backText.setFillColor(sf::Color::Black);
	backText.setCharacterSize(48);
	backText.setPosition(50 + 5, backButton.getPosition().y - 10);

	//-------------- ok ----------------------------
	sf::RectangleShape okButton(sf::Vector2f(200.f, 50.f));
	okButton.setPosition(window.getSize().x - 200, window.getSize().y - 100);
	okButton.setFillColor(sf::Color::Green);

	sf::Text okText;
	okText.setFont(font);
	okText.setString("Connect");
	okText.setStyle(sf::Text::Regular);
	okText.setFillColor(sf::Color::Black);
	okText.setCharacterSize(48);
	okText.setPosition(window.getSize().x - 200 + 5, okButton.getPosition().y - 10);

	window.clear();

	while (window.isOpen())
	{
		sf::Event event;
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
		ReceiveData();
		if (GTFO) {
			window.close();
			disconnected = true;
		}
		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				exitGame = disconnected = true;
				window.close();
				break;
			case sf::Event::MouseMoved:
			{
				//login
				if (loginButton.getGlobalBounds().contains(mousePosF))
					loginButton.setFillColor(grey);
				else loginButton.setFillColor(sf::Color::White);
				//sign
				if (signButton.getGlobalBounds().contains(mousePosF))
					signButton.setFillColor(grey);
				else signButton.setFillColor(sf::Color::White);

			}
			break;
			case sf::Event::MouseButtonPressed:
			{
				if (loginButton.getGlobalBounds().contains(mousePosF) && !login && !sign)
				{
					login = true;
					username = true;
					usernameButton.setFillColor(grey);
					titleText.setString("Login");

					break;
				}
				else if (signButton.getGlobalBounds().contains(mousePosF) && !sign && !login)
				{
					sign = true;
					username = true;
					titleText.setString("Sign");
					break;
				}
				else if (backButton.getGlobalBounds().contains(mousePosF))
				{
					login = sign = username = passwordConnection = mail = false;
					userInput.clear();
					mailInput.clear();
					passInput.clear();
					userText.setString("");
					mailText.setString("");
					passText.setString("");
					break;
				}
				else if (okButton.getGlobalBounds().contains(mousePosF))
				{
					sf::Packet toSend;
					if (login || sign) {
						if (login)
							toSend << LOGIN << packetID << userInput << passInput;
						else if (sign) toSend << SIGNUP << packetID << userInput << passInput << mailInput;

						myPlayer->resending.insert(std::make_pair(packetID, toSend));

						packetID++;
						toSend.clear();
						window.close();
					}
					break;
				}

				else if (usernameButton.getGlobalBounds().contains(mousePosF) && (login || sign))
				{
					username = true;
					passwordConnection = mail = false;
					break;
				}
				else if (passConnectionButton.getGlobalBounds().contains(mousePosF) && (login || sign))
				{
					passwordConnection = true;
					username = mail = false;
					break;
				}
				else if (mailButton.getGlobalBounds().contains(mousePosF) && sign)
				{
					mail = true;
					username = passwordConnection = false;
					break;
				}

			}
			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Return) {

					if (username) {
						username = false;
						passwordConnection = true;
						myPlayer->nickname = userInput;
					}
					else if (passwordConnection) {
						passwordConnection = false;
						if (sign)
							mail = true;
					}
					else if (mail) {
						mail = false;
						break;
					}
				}
			}
			break;
			case sf::Event::TextEntered:
			{
				if (event.text.unicode == '\b') {
					if (username) {
						if (userInput.getSize() > 0)
							userInput.erase(userInput.getSize() - 1, 1);
						userText.setString(userInput);
					}
					else if (passwordConnection) {
						if (passInput.getSize() > 0)
							passInput.erase(passInput.getSize() - 1, 1);
						passText.setString(passInput);
					}
					else if (mail) {
						if (mailInput.getSize() > 0)
							mailInput.erase(mailInput.getSize() - 1, 1);
						mailText.setString(mailInput);
					}
				}
				else if (event.text.unicode < 128)
				{
					if (username) {
						userInput += event.text.unicode;
						userText.setString(userInput);
					}
					else if (passwordConnection) {
						passInput += event.text.unicode;
						passText.setString(passInput);
					}
					else if (mail) {
						mailInput += event.text.unicode;
						mailText.setString(mailInput);
					}
				}
			}
			break;
			}
		}
		window.clear();

		if (username) usernameButton.setFillColor(grey);
		else usernameButton.setFillColor(sf::Color::White);
		if (passwordConnection) passConnectionButton.setFillColor(grey);
		else passConnectionButton.setFillColor(sf::Color::White);
		if (mail) mailButton.setFillColor(grey);
		else mailButton.setFillColor(sf::Color::White);

		if (!login && !sign) {
			username = passwordConnection = mail = false;
			userInput.clear();
			mailInput.clear();
			passInput.clear();
			userText.setString("");
			mailText.setString("");
			passText.setString("");
			window.draw(titleText);
			window.draw(loginButton);
			window.draw(loginText);
			window.draw(signButton);
			window.draw(signText);
		}
		else if (login) {
			window.draw(titleText);
			window.draw(userNameText);
			window.draw(usernameButton);
			window.draw(userText);
			window.draw(passwordConecText);
			window.draw(passConnectionButton);
			window.draw(passText);
			window.draw(backButton);
			window.draw(backText);
			window.draw(okButton);
			window.draw(okText);
		}
		else if (sign) {
			window.draw(titleText);
			window.draw(userNameText);
			window.draw(usernameButton);
			window.draw(userText);
			window.draw(passwordConecText);
			window.draw(passConnectionButton);
			window.draw(passText);
			window.draw(mailConectText);
			window.draw(mailButton);
			window.draw(mailText);
			window.draw(backButton);
			window.draw(backText);
			window.draw(okButton);
			window.draw(okText);
		}

		window.display();
	}

}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "PILLAPILLA GAME - Client: " + myPlayer->nickname);

	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();

		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}

		if (clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS) { //si es més gran envio, restart rellotge, al rebre he de borrarlos de la llista

			sf::Packet packet;
			if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {
				packet << TRY_POSITION << packetID << myPlayer->ID << myPlayer->IDPartida << myPlayer->MapAccumMovements.find(idMovements)->first << myPlayer->MapAccumMovements.find(idMovements)->second; //montar un paquet amb totes les posicions acumulades

				status = socket.send(packet, "localhost", PORT);
				packetID++;
				idMovements++; //una avegada enviem ja puc incrementar, per tant nomes es posara una vegada al resending
				clockPositions.restart();
			}

			//comprovar collisio, si es detecta enviar al server per validacio
			packet.clear();
			for (std::map<int32_t, InterpolationAndStuff>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
				if (it->second.lastPos.x <= myPlayer->position.x + RADIUS_SPRITE && it->second.lastPos.x >= myPlayer->position.x - RADIUS_SPRITE && it->second.lastPos.y <= myPlayer->position.y + RADIUS_SPRITE && it->second.lastPos.y >= myPlayer->position.y - RADIUS_SPRITE) {
					packet << TRY_COLLISION_OPPONENT << packetID <<  myPlayer->ID << myPlayer->IDPartida << it->first;
					status = socket.send(packet, "localhost", PORT);
					packetID++;
				}
			}
		}

		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				exitGame = true;
				break;
			case  sf::Event::KeyPressed: //el moviment en aquesta versio es per celes
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) //moure personatge esquerra
				{
					//if (!notMove)
					myPlayer->position.x -= PIXELSTOMOVE;
					if (myPlayer->MapAccumMovements.find(idMovements) == myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ -PIXELSTOMOVE,0 },  Position{ myPlayer->position.x - PIXELSTOMOVE,myPlayer->position.y } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += -PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += -PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += 0;
					}
					//idMovements++;

				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) //moure personatge dreta
				{
					//if (!notMove)
					myPlayer->position.x += PIXELSTOMOVE;


					if (myPlayer->MapAccumMovements.find(idMovements) == myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ +PIXELSTOMOVE,0 },  Position{ myPlayer->position.x + PIXELSTOMOVE,myPlayer->position.y } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += 0;
					}
					//idMovements++;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) //moure personatge dalt
				{
					//if (!notMove)
					myPlayer->position.y -= PIXELSTOMOVE;


					if (myPlayer->MapAccumMovements.find(idMovements) == myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ 0,-PIXELSTOMOVE },  Position{ myPlayer->position.x, myPlayer->position.y - PIXELSTOMOVE } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += -PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += -PIXELSTOMOVE;
					}
					//idMovements++;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) //moure personatge baix
				{
					//if (!notMove)
					myPlayer->position.y += PIXELSTOMOVE;

					if (myPlayer->MapAccumMovements.find(idMovements) == myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ 0, PIXELSTOMOVE },  Position{ myPlayer->position.x, myPlayer->position.y + PIXELSTOMOVE } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += PIXELSTOMOVE;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += PIXELSTOMOVE;
					}
					//idMovements++;
				}
			default:
				break;

			}
		}

		//clearing window and drawing again
		window.clear();

		for (int8_t i = 0; i < NUMBER_ROWS_COLUMNS; i++)
		{
			for (int8_t j = 0; j < NUMBER_ROWS_COLUMNS; j++)
			{
				sf::RectangleShape rectBlanco(sf::Vector2f(SIZE_CELL, SIZE_CELL));
				rectBlanco.setFillColor(greyFosc);

				if (i % 2 == 0)
				{
					if (j % 2 == 0)
					{
						rectBlanco.setPosition(sf::Vector2f(i*SIZE_CELL, j*SIZE_CELL));
						window.draw(rectBlanco);
					}
				}
				else
				{
					if (j % 2 == 1)
					{
						rectBlanco.setPosition(sf::Vector2f(i*SIZE_CELL, j*SIZE_CELL));
						window.draw(rectBlanco);
					}
				}
			}
		}

		//draw the player circle
		sf::CircleShape shapePlayer(RADIUS_SPRITE);
		//std::cout << shapePlayer.getGlobalBounds().height << std::endl;
		if (!myPlayer->laParo)
			shapePlayer.setFillColor(sf::Color::Green);
		else shapePlayer.setFillColor(sf::Color::Magenta);

		sf::Vector2f positionPlayer(myPlayer->position.x, myPlayer->position.y);
		shapePlayer.setPosition(positionPlayer);

		window.draw(shapePlayer);


		//draw the opponents circle
		sf::CircleShape shapeOpponent(RADIUS_SPRITE);


		for (std::map<int32_t, InterpolationAndStuff>::iterator it = opponents.begin(); it != opponents.end(); ++it) {

			if (!it->second.middlePositions.empty()) {
				Position temp = it->second.middlePositions.front(); //agafem valor
				it->second.middlePositions.pop(); //borrem de la cua
				shapeOpponent.setPosition(temp.x, temp.y);
				if (!it->second.laPara)
					shapeOpponent.setFillColor(sf::Color::Blue);
				else shapeOpponent.setFillColor(sf::Color::Red);
				window.draw(shapeOpponent);
			}
			else {
				shapeOpponent.setPosition(it->second.lastPos.x, it->second.lastPos.y);
				if (!it->second.laPara)
					shapeOpponent.setFillColor(sf::Color::Blue);
				else shapeOpponent.setFillColor(sf::Color::Red);
				window.draw(shapeOpponent);

			}

		}


		sf::RectangleShape wall(sf::Vector2f(SIZE_CELL, SIZE_CELL));
		wall.setFillColor(sf::Color::Yellow);

		for (std::vector<Position>::iterator it = myWalls->obstaclesMap.begin(); it != myWalls->obstaclesMap.end(); ++it) {
			wall.setPosition(sf::Vector2f(it->x * SIZE_CELL, it->y * SIZE_CELL));
			window.draw(wall);
		}

		window.display();
	}

}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	socket.setBlocking(false);

	myPlayer = new Jugador();
	myWalls = new Walls();
	//partidas.insert(std::make_pair(1, PartidaClient{ 1,"La 1ra Partida Loko", 4, 1 }));
	//partidas.insert(std::make_pair(2, PartidaClient{ 2,"Brrr", 3, 2 }));
	//initial connection
	c.restart();
	ConnectionWithServer(); 

	if (!disconnected) {
		do {
			Lobby();
			if (!exitGame) {
				clockPositions.restart(); //a partir daqui ja es pot acabar de moure per tant fem un reset del rellotje
				GameManager();
			}
		} while (!disconnected);
	}
	opponents.clear();
	socket.unbind();
	system("exit");

	return 0;
}







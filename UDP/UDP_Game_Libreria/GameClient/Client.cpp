//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <GlobalValues.h>

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;

sf::UdpSocket socket;
sf::Socket::Status status;
int32_t packetID = 1;
sf::Clock c;

Player * myPlayer;
//std::map <int32_t, Position> opponents;
std::map <int32_t, InterpolationAndStuff> opponents;
sf::Clock clockPositions;
int32_t idMovements = 1;
int32_t idUltimMoviment = 0;
Walls * myWalls; 
int32_t winner;
sf::RenderWindow window;
bool alreadySaidWinner, GTFO, once, create, join, name, password, maxNum, exitGame, writePassword, disconnected, sortByName, sortByConnected, sortByMax;
std::map <int8_t, Partida> partidas;
std::map<std::string, ListButtons> listadoPartidasPorNombre;
std::map<int8_t, ListButtons> listadoPartidasPorNumConnectados;
std::map<int8_t, ListButtons> listadoPartidasPorMaxPlayers;
std::vector<ListButtons> vectorListaPartidas;

sf::Color grey = sf::Color(169, 169, 169);
sf::Color greyFosc = sf::Color(49, 51, 53);

void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";
	window.create(sf::VideoMode(500, 500), "Lobby", sf::Style::Default);

	sf::RectangleShape inputButton(sf::Vector2f(300.f, 60.f));
	inputButton.setPosition(window.getSize().x/2/2, window.getSize().y/2);
	inputButton.setFillColor(sf::Color::White);

	sf::Font font;
	if (!font.loadFromFile("calibri.ttf"))
		std::cout << "Can't find the font file" << std::endl;

	sf::Text startText;
	startText.setFont(font);
	startText.setStyle(sf::Text::Regular);
	startText.setString("Username: ");
	startText.setFillColor(sf::Color::White);
	startText.setCharacterSize(48);
	startText.setPosition(window.getSize().x / 2/2, window.getSize().y / 2 - 100);

	sf::String playerInput;
	sf::Text playerText("", font, 48);
	playerText.setPosition(window.getSize().x / 2/2, window.getSize().y / 2);
	playerText.setFillColor(sf::Color::Black);
	window.clear();
	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					window.close();
					exitGame = disconnected = true;
					break;
				case sf::Event::KeyPressed:
				{
					if (event.key.code == sf::Keyboard::Return) {
						sf::Packet packet;
						myPlayer->nickname = playerText.getString();
						packet << HELLO << packetID << myPlayer->nickname;
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						packet.clear();	
						window.close();
						break;
					}
				}
				break;
				case sf::Event::TextEntered:
				{
					if (event.text.unicode == '\b') {
						if (playerInput.getSize() > 0)
							playerInput.erase(playerInput.getSize() - 1, 1);
						playerText.setString(playerInput);
					}
					else if (event.text.unicode < 128)
					{
						playerInput += event.text.unicode;
						playerText.setString(playerInput);
					}
				}
				break;
			}
				
		}
		window.draw(startText);
		window.draw(inputButton);
		window.draw(playerText);
		window.display();
	}
	/*std::cout << "Type your nickname: ";
	std::getline(std::cin, myPlayer->nickname);*/
	/*sf::Packet packet;
	packet << HELLO << packetID << myPlayer->nickname;
	myPlayer->resending.insert(std::make_pair(packetID, packet));
	packetID++;
	packet.clear();*/
}


void Resend() {
	
	for (std::map<int32_t, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
		status = socket.send(msg->second, "localhost", PORT);
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

void SendACK(int cmd, int32_t pID) {
	sf::Packet packet;
	std::string com;

	if (cmd == ACK_DISCONNECTION) {
		com = "DISCONNECTION";
	}
	else if (cmd == ACK_NEW_CONNECTION) {
		com = "NEW_CONNECTION";
	}
	else if (cmd == ACK_PING) {
		com = "ACK_PING";
	}
	else if (cmd == ACK_QUI_LA_PILLA) {
		com = "ACK_QUI_LA_PILLA";
	}
	else if (cmd == ACK_WINNER) {
		com = "ACK_WINNER";
	}

	packet << cmd << pID << myPlayer->ID;
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
			if (cmd == ID_ALREADY_TAKEN) {
				std::cout << "This ID it's already used." << std::endl;
				//ConnectionWithServer();
				GTFO = true;
				
			}
			else if (cmd == GAMESTARTED) {
				//SendACK(ACK_GAMESTARTED, packetIDRecived);
				std::cout << "The game PILLAPILLA started." << std::endl;
			}
			else if (cmd == ACK_HELLO) {
				if (myPlayer->ID == 0) {
					int32_t numOfOpponents = 0;
					packet >> myPlayer->ID >> myPlayer->position >> numOfOpponents;

					if (numOfOpponents > 0) {
						//treiem del packet la ID i la pos de cada oponent
						for (int i = 0; i < numOfOpponents; i++) {
							//int8_t opponentId;
							Position oPos;
							packet >> opponentId >> oPos;
							opponents.insert(std::make_pair(opponentId, InterpolationAndStuff{ oPos, oPos, false }));
						}
					}
					if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
						myPlayer->resending.erase(packetIDRecived);
					}
					std::cout << "WELCOME! " << " Client ID: " << std::to_string(myPlayer->ID) << " Initial Position: " << std::to_string(myPlayer->position.x) << ", " << std::to_string(myPlayer->position.y) << std::endl;
				}
			}

			else if (cmd == NEW_CONNECTION) {
				packet >> opponentId;
				if (opponents.find(opponentId) == opponents.end()) {
					Position pos;
					packet >> pos;
					std::cout << "A new opponent connected. ID: " << std::to_string(opponentId) << " Position: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << std::endl;
					opponents.insert(std::make_pair(opponentId, InterpolationAndStuff{ pos, pos, false }));
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
	createText.setPosition(window.getSize().x / 2 / 2+20, createButton.getPosition().y - 10);

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
	exitText.setPosition(window.getSize().x / 2 / 2 + 50, exitButton.getPosition().y -10);

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
	nameText.setPosition(window.getSize().x / 2 / 2, 200-10);
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
	passText.setPosition(window.getSize().x / 2 / 2, 300-10);
	passText.setFillColor(sf::Color::Black);

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
	numText.setPosition(window.getSize().x / 2 / 2, 400-10);
	numText.setFillColor(sf::Color::Black);

	//-------------- Join: List ----------------------------
	int16_t positionY = 100;
	for (std::map<int8_t, Partida>::iterator it = partidas.begin(); it != partidas.end(); ++it) {

		//std::string str = it->second.name + "\t" + std::to_string(it->second.numPlayersConnected) + "\t" + std::to_string(it->second.maxPlayers);

		sf::RectangleShape button(sf::Vector2f(1150, 50.f));
		button.setPosition(50, positionY);
		button.setFillColor(sf::Color::Transparent);

		sf::Text name;
		name.setFont(font);
		name.setStyle(sf::Text::Regular);
		name.setString(it->second.name);
		name.setFillColor(sf::Color::White);
		name.setCharacterSize(48);
		name.setPosition(button.getPosition().x + 20, button.getPosition().y - 10);

		sf::Text conn;
		conn.setFont(font);
		conn.setStyle(sf::Text::Regular);
		conn.setString(std::to_string(it->second.numPlayersConnected));
		conn.setFillColor(sf::Color::White);
		conn.setCharacterSize(48);
		conn.setPosition(button.getPosition().x + 900, button.getPosition().y - 10);

		sf::Text max;
		max.setFont(font);
		max.setStyle(sf::Text::Regular);
		max.setString(std::to_string(it->second.maxPlayers));
		max.setFillColor(sf::Color::White);
		max.setCharacterSize(48);
		max.setPosition(conn.getPosition().x + conn.getLocalBounds().width + 150, button.getPosition().y - 10);

		/*listadoPartidasPorNombre.insert(std::make_pair(it->second.name, ListButtons{ name, conn, max, button }));
		listadoPartidasPorNumConnectados.insert(std::make_pair(it->second.numPlayersConnected, ListButtons{ name, conn, max, button }));
		listadoPartidasPorMaxPlayers.insert(std::make_pair(it->second.maxPlayers, ListButtons{ name, conn, max, button }));*/

		vectorListaPartidas.push_back(ListButtons{ name, conn, max, button });

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

	/*for (int i = 0; i < listadoPartidasPorMaxPlayers.size(); i++) {
		std::string str = listadoPartidasPorMaxPlayers[i].name.getString();
		std::cout << str << std::endl;
	}*/

	while (window.isOpen())
	{
		sf::Event Event;
		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
		
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
					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						if(vectorListaPartidas[i].rect.getGlobalBounds().contains(mousePosF))
							vectorListaPartidas[i].rect.setFillColor(grey);
						else vectorListaPartidas[i].rect.setFillColor(sf::Color::Transparent);
					}
					/*if (sortByName) {
						for (std::map<std::string, ListButtons>::iterator it = listadoPartidasPorNombre.begin(); it != listadoPartidasPorNombre.end(); ++it) {
							if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
								it->second.rect.setFillColor(grey);
							}
							else it->second.rect.setFillColor(sf::Color::Transparent);
						}
					}
					else if (sortByConnected) {
						for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorNumConnectados.begin(); it != listadoPartidasPorNumConnectados.end(); ++it) {
							if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
								it->second.rect.setFillColor(grey);
							}
							else it->second.rect.setFillColor(sf::Color::Transparent);
						}
					}
					else if (sortByMax) {
						for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorMaxPlayers.begin(); it != listadoPartidasPorMaxPlayers.end(); ++it) {
							if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
								it->second.rect.setFillColor(grey);
							}
							else it->second.rect.setFillColor(sf::Color::Transparent);
						}
					}*/
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
					sortByMax = true;
					okText.setString("Join");
					break;
				}
				else if (backButton.getGlobalBounds().contains(mousePosF))
				{
					create = join = name = password = maxNum = writePassword = false;

					break;
				}
				else if (okButton.getGlobalBounds().contains(mousePosF))
				{
					if (create)
						window.close();
					else if (join)
						writePassword = true;
					break;
				}
				else if (exitButton.getGlobalBounds().contains(mousePosF) && !create && !join)
				{
					window.close();
					exitGame = disconnected = true;
					break;
				}
				else if (sortNameButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByName = true;
					sortByMax = sortByConnected = false;
					std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByName);
					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						std::string str = vectorListaPartidas[i].name.getString();
						std::cout << str << std::endl;
					}
					break;
				}
				else if (sortConButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByConnected = true;
					sortByName = sortByMax = false;
					std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnection);
					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						std::string str = vectorListaPartidas[i].name.getString();
						std::cout << str << std::endl;
					}
					break;
				}
				else if (sortMaxButton.getGlobalBounds().contains(mousePosF) && join)
				{
					sortByMax = true;
					sortByName = sortByConnected = false;
					std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNum);
					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						std::string str = vectorListaPartidas[i].name.getString();
						std::cout << str << std::endl;
					}
					break;
				}
				if (join) {

					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
						if (vectorListaPartidas[i].rect.getGlobalBounds().contains(mousePosF)) {
							//ensenyar introduccio password
							//unirme a aquella partida si la password es correcte
							window.close();
							break;
						}
							
					}

					//if (sortByName) {
					//	for (std::map<std::string, ListButtons>::iterator it = listadoPartidasPorNombre.begin(); it != listadoPartidasPorNombre.end(); ++it) {
					//		if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
					//			//ensenyar introduccio password
					//			//unirme a aquella partida si la password es correcte
					//			window.close();
					//			break;
					//		}
					//	}
					//}
					//else if (sortByConnected) {
					//	for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorNumConnectados.begin(); it != listadoPartidasPorNumConnectados.end(); ++it) {
					//		if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
					//			//ensenyar introduccio password
					//			//unirme a aquella partida si la password es correcte
					//			window.close();
					//			break;
					//		}
					//	}
					//}
					//else if (sortByMax) {
					//	for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorMaxPlayers.begin(); it != listadoPartidasPorMaxPlayers.end(); ++it) {
					//		if (it->second.rect.getGlobalBounds().contains(mousePosF)) {
					//			//ensenyar introduccio password
					//			//unirme a aquella partida si la password es correcte
					//			window.close();
					//			break;
					//		}
					//	}
					//}
				}
			}
			break;
			case sf::Event::TextEntered:
			{	
				//si clico el backspace/borrar/retroceso
				if (Event.text.unicode == '\b' && name) {
					if(nameInput.getSize() > 0)
						nameInput.erase(nameInput.getSize() - 1, 1);
					nameText.setString(nameInput);
				}
				else if (Event.text.unicode == '\b' && password) {
					if(passInput.getSize())
						passInput.erase(nameInput.getSize() - 1, 1);
					passText.setString(passInput);
				}
				else if (Event.text.unicode == '\b' && maxNum) {
					if (numInput.getSize())
						numInput.erase(numInput.getSize() - 1, 1);
					numText.setString(numInput);
				}
				//si clico algo q no sigui borrar
				else if (Event.text.unicode < 128 && name)
				{
					nameInput += Event.text.unicode;
					nameText.setString(nameInput);
				}
				else if (Event.text.unicode < 128 && password) {
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
						createNameButton.setFillColor(sf::Color::White);
						createPassButton.setFillColor(sf::Color(169, 169, 169));
						//enviar nameText.getString();
					}
					else if (password) {
						password = false;
						maxNum = true;
						createPassButton.setFillColor(sf::Color::White);
						createMaxButton.setFillColor(sf::Color(169, 169, 169));
						//enviar passText.getString();
					}
					else if (maxNum) {
						maxNum = false;
						createMaxButton.setFillColor(sf::Color::White);
						//enviar numText.getString();
						break;
					}
					break;
				}
			}
			break;
			}
		}
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
		//NO FUNCIONA CORRECTAMENT! -> IMPRIMEIX PERO NO IMPRIMEIX SEGONS L'ORDRE QUE TOCARIA...NO HO ENTENC 
		else if (join) { //mostro lobby de unirse partida
			//if (sortByName) {
			//	sortNameButton.setFillColor(sf::Color::Blue);
			//	for (std::map<std::string, ListButtons>::iterator it = listadoPartidasPorNombre.begin(); it != listadoPartidasPorNombre.end(); ++it) {
			//		window.draw(it->second.rect);
			//		window.draw(it->second.name);
			//		window.draw(it->second.connected);
			//		window.draw(it->second.numMax);
			//	}
			//}
			//if (sortByConnected) {
			//	sortConButton.setFillColor(sf::Color::Blue);
			//	for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorNumConnectados.begin(); it != listadoPartidasPorNumConnectados.end(); ++it) {
			//		window.draw(it->second.rect);
			//		window.draw(it->second.name);
			//		window.draw(it->second.connected);
			//		window.draw(it->second.numMax);					
			//	}
			//}
			//if (sortByMax) {
			//	sortMaxButton.setFillColor(sf::Color::Blue);
			//	/*std::string str = listadoPartidasPorMaxPlayers.begin()->second.numMax.getString();
			//	std::cout << str << std::endl;*/
			//	for (std::map<int8_t, ListButtons>::iterator it = listadoPartidasPorMaxPlayers.begin(); it != listadoPartidasPorMaxPlayers.end(); it++) {

			//		window.draw(it->second.rect);
			//		window.draw(it->second.name);
			//		window.draw(it->second.connected);
			//		window.draw(it->second.numMax);
			//	}
			//}
			if(sortByMax) sortMaxButton.setFillColor(sf::Color::Blue);
			else if (sortByConnected) sortConButton.setFillColor(sf::Color::Blue);
			else if (sortByName) sortNameButton.setFillColor(sf::Color::Blue);

			for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
				window.draw(vectorListaPartidas[i].rect);
				window.draw(vectorListaPartidas[i].name);
				window.draw(vectorListaPartidas[i].connected);
				window.draw(vectorListaPartidas[i].numMax);
			}

			//Capçalera
			window.draw(capName);
			window.draw(capCon);
			window.draw(capMax);
			//back
			window.draw(backButton);
			window.draw(backText);
			//ok
			window.draw(okButton);
			window.draw(okText);
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
		window.display();
	}
}

void GameManager() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "PILLAPILLA GAME - Client: " + myPlayer->nickname);

	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();
			
		if (GTFO) {	
			window.close();
			disconnected = true;
		}
		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}

		if (clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS) { //si es més gran envio, restart rellotge, al rebre he de borrarlos de la llista

			sf::Packet packet; 
			if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {
				packet << TRY_POSITION << packetID << myPlayer->ID << myPlayer->MapAccumMovements.find(idMovements)->first << myPlayer->MapAccumMovements.find(idMovements)->second; //montar un paquet amb totes les posicions acumulades

				status = socket.send(packet, "localhost", PORT);
				packetID++;
				idMovements++; //una avegada enviem ja puc incrementar, per tant nomes es posara una vegada al resending
				clockPositions.restart();
			}

			//comprovar collisio, si es detecta enviar al server per validacio
			packet.clear();
			for (std::map<int32_t, InterpolationAndStuff>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
				if (it->second.lastPos.x <= myPlayer->position.x + RADIUS_SPRITE && it->second.lastPos.x >= myPlayer->position.x - RADIUS_SPRITE && it->second.lastPos.y <= myPlayer->position.y + RADIUS_SPRITE && it->second.lastPos.y >= myPlayer->position.y - RADIUS_SPRITE) {
					packet << TRY_COLLISION_OPPONENT << packetID << myPlayer->ID << it->first;
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
				disconnected = true;
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
		if(!myPlayer->laParo)
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
				if(!it->second.laPara)
					shapeOpponent.setFillColor(sf::Color::Blue);
				else shapeOpponent.setFillColor(sf::Color::Red);
				window.draw(shapeOpponent);
			}
			else{
				shapeOpponent.setPosition(it->second.lastPos.x, it->second.lastPos.y);
				if (!it->second.laPara)
					shapeOpponent.setFillColor(sf::Color::Blue);
				else shapeOpponent.setFillColor(sf::Color::Red);
				window.draw(shapeOpponent);
				
			}

		}
			

		sf::RectangleShape wall(sf::Vector2f(SIZE_CELL, SIZE_CELL));
		wall.setFillColor(sf::Color::Yellow);

		for (std::vector<Position>::iterator it = myWalls->obstaclesMap.begin(); it !=  myWalls->obstaclesMap.end(); ++it) {
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

	myPlayer = new Player();
	myWalls = new Walls();
	partidas.insert(std::make_pair(1, Partida{ "La 1ra Partida Loko", "LOL", 4, 1 }));
	partidas.insert(std::make_pair(2, Partida{ "Brrr", "LOL", 3, 2 }));
	//initial connection
	ConnectionWithServer();
	do{ //aquest bucle no se si esta be
		Lobby();
		if (!exitGame) {
			clockPositions.restart(); //a partir daqui ja es pot acabar de moure per tant fem un reset del rellotje
			GameManager();
		}
	} while (!disconnected);
	opponents.clear();
	socket.unbind();
	system("exit");

	return 0;
}


/*std::string nomPartida, jugadorsConnectats, maxjugadors, str, del = "\t";
str = it->first;
//------partim el string donat en multiples strings
size_t pos = 0;
pos = str.find(del);
nomPartida = str.substr(0, pos);
str.erase(0, pos + del.length());
pos = str.find(del);
jugadorsConnectats = str.substr(0, pos);
str.erase(0, pos + del.length());
maxjugadors = str;
//----------------------------------------------------------

sf::Text name;
name.setFont(font);
name.setStyle(sf::Text::Regular);
name.setString(std::to_string(i) + ". " + nomPartida);
name.setFillColor(sf::Color::White);
name.setCharacterSize(48);
name.setPosition(it->second.getPosition().x + 20, it->second.getPosition().y - 10);
i++;

sf::Text conn;
conn.setFont(font);
conn.setStyle(sf::Text::Regular);
conn.setString(jugadorsConnectats);
conn.setFillColor(sf::Color::White);
conn.setCharacterSize(48);
conn.setPosition(it->second.getPosition().x + 900, it->second.getPosition().y - 10);

sf::Text max;
max.setFont(font);
max.setStyle(sf::Text::Regular);
max.setString(maxjugadors);
max.setFillColor(sf::Color::White);
max.setCharacterSize(48);
max.setPosition(conn.getPosition().x + conn.getLocalBounds().width + 150, it->second.getPosition().y - 10);

window.draw(it->second);
window.draw(name);
window.draw(conn);
window.draw(max);*/







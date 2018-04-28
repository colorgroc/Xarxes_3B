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
std::map <int32_t, Interpolation> opponents;
sf::Clock clockPositions;
int32_t idMovements = 1;
int32_t idUltimMoviment = 0;
Walls * myWalls; 
//bool notMove = false;

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
			else if (cmd == ACK_HELLO) {
				if (myPlayer->ID == 0) {
					int32_t numOfOpponents = 0;
					packet >> myPlayer->ID >> myPlayer->position >> numOfOpponents;

					if (numOfOpponents > 0) {
						//treiem del packet la ID i la pos de cada oponent
						for (int i = 0; i < numOfOpponents; i++) {
							int32_t oID;
							Position oPos;
							packet >> oID >> oPos;
							opponents.insert(std::make_pair(oID, Interpolation{ oPos, oPos }));
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
					opponents.insert(std::make_pair(opponentId, Interpolation{ pos, pos }));
				}
				SendACK(ACK_NEW_CONNECTION, packetIDRecived);
			}
			else if (cmd == OK_POSITION) {

				//solucionar problemes d'ordre de paquets
				//si hi han id mes petits a la llista del client es poden borrar ja que es va a la poscio m�s allunyada
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
		}

	} 
	
	packet.clear();
}

bool CheckCollisionPlayerOpponent() {
	bool collision = false;
	for (std::map<int32_t, Interpolation>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
		if (it->second.lastPos.x <= myPlayer->position.x + 15 && it->second.lastPos.x >= myPlayer->position.x - 15 && it->second.lastPos.y <= myPlayer->position.y + 15 && it->second.lastPos.y >= myPlayer->position.y - 15) {
			std::cout << "Collision With Opponent" << std::endl;
			collision = true;
		}
	}
	return collision;
}

void GameManager() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Traffic Game - Client: " + myPlayer->nickname);

	while (window.isOpen())
	{
		sf::Event event;
		ReceiveData();
		CheckCollisionPlayerOpponent();
		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
			Resend();
			c.restart();
		}

		if (clockPositions.getElapsedTime().asMilliseconds() > SEND_ACCUMMOVEMENTS) { //si es m�s gran envio, restart rellotge, al rebre he de borrarlos de la llista

			sf::Packet packet; 
			if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {
				packet << TRY_POSITION << packetID << myPlayer->ID << myPlayer->MapAccumMovements.find(idMovements)->first << myPlayer->MapAccumMovements.find(idMovements)->second; //montar un paquet amb totes les posicions acumulades

				status = socket.send(packet, "localhost", PORT);
				packetID++;
				idMovements++; //una avegada enviem ja puc incrementar, per tant nomes es posara una vegada al resending
				clockPositions.restart();
			}	
		}
	
		//inputs game
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				socket.unbind();
				window.close();
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
				sf::Color grey = sf::Color(49, 51, 53);
				rectBlanco.setFillColor(grey);

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
		shapePlayer.setFillColor(sf::Color::Green);

		sf::Vector2f positionPlayer(myPlayer->position.x, myPlayer->position.y);
		shapePlayer.setPosition(positionPlayer);

		window.draw(shapePlayer);


		//draw the opponents circle
		sf::CircleShape shapeOpponent(RADIUS_SPRITE);
		shapeOpponent.setFillColor(sf::Color::Red);


		for (std::map<int32_t, Interpolation>::iterator it = opponents.begin(); it != opponents.end(); ++it) {

			if (!it->second.middlePositions.empty()) {
				Position temp = it->second.middlePositions.front(); //agafem valor
				it->second.middlePositions.pop(); //borrem de la cua
				shapeOpponent.setPosition(temp.x, temp.y);
				window.draw(shapeOpponent);
			}
			else{
				shapeOpponent.setPosition(it->second.lastPos.x, it->second.lastPos.y);
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


void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";
	std::cout << "Type your nickname: ";
	std::getline(std::cin, myPlayer->nickname);
	sf::Packet packet;
	packet << HELLO << packetID << myPlayer->nickname; 
	myPlayer->resending.insert(std::make_pair(packetID, packet));
	packetID++;
	packet.clear();
}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	socket.setBlocking(false);

	myPlayer = new Player();
	myWalls = new Walls();

	//initial connection
	ConnectionWithServer();

	clockPositions.restart(); //a partir daqui ja es pot acabar de moure per tant fem un reset del rellotje
	GameManager();

	opponents.clear();
	socket.unbind();
	system("exit");

	return 0;
}










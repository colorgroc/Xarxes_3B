//TALLER 6 - ANNA PONCE I MARC SEGARRA

#include <GlobalValues.h>

//comandos
int8_t HELLO = 0;
int8_t ACK_HELLO = 1;
int8_t NEW_CONNECTION = 2;
int8_t ACK_NEW_CONNECTION = 3;
int8_t DISCONNECTION = 4;
int8_t ACK_DISCONNECTION = 5;
int8_t PING = 6;
int8_t ACK_PING = 7;
int8_t TRY_POSITION = 8;
int8_t OK_POSITION = 9;
int8_t REFRESH_POSITIONS = 10;
int8_t ACK_REFRESH_POSITIONS = 11;

sf::IpAddress serverIP = "localhost";
unsigned short serverPORT = PORT;

sf::UdpSocket socket;
sf::Socket::Status status;
int8_t packetID = 1;
sf::Clock c;

Player * myPlayer;
std::map <int8_t, Position> opponents;

sf::Clock clockPositions;
int8_t idMovements;

void Resend() {
	//posar mutex??

	for (std::map<int8_t, sf::Packet>::iterator msg = myPlayer->resending.begin(); msg != myPlayer->resending.end(); ++msg) {
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

void SendACK(int8_t cmd, int8_t pID) {
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
	else if (cmd == ACK_REFRESH_POSITIONS) {
		com = "ACK_REFRESH_POSITIONS";
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
	int8_t cmd = 0;
	int8_t opponentId = 0;
	int8_t packetIDRecived = 0;

	status = socket.receive(packet, serverIP, serverPORT);

	if (status == sf::Socket::Done) {
		packet >> cmd >> packetIDRecived;

		if (cmd == PING) {
			SendACK(ACK_PING, packetIDRecived);
		}
		else if (cmd == ACK_HELLO) {
			//std::cout << "ACK_HELLO recived." << std::endl;
			if (myPlayer->ID == 0) {
				int8_t numOfOpponents = 0;
				packet >> myPlayer->ID >> myPlayer->position >> numOfOpponents;
				
				myPlayer->position = CellToPixel(myPlayer->position.x, myPlayer->position.y);

				if (numOfOpponents > 0) {
					//treiem del packet la ID i la pos de cada oponent
					for (int i = 0; i < numOfOpponents; i++) {
						int8_t oID;
						Position oPos;
						packet >> oID >> oPos;
						opponents.insert(std::make_pair(oID, oPos));
					}
				}
				if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end()) {
					myPlayer->resending.erase(packetIDRecived);
				}
				std::cout << "WELCOME! " << " Client ID: " << std::to_string(myPlayer->ID) << " Initial Position: " << std::to_string(myPlayer->position.x) << ", " << std::to_string(myPlayer->position.y) << std::endl;
			}
		}

		//std::cout << std::to_string(opponentId) << std::endl;
		else if (cmd == NEW_CONNECTION) {
			packet >> opponentId;
			if (opponents.find(opponentId) == opponents.end()) {
				Position pos;
				packet >> pos;
				std::cout << "A new opponent connected. ID: " << std::to_string(opponentId) << " Position: " << std::to_string(pos.x) << ", " << std::to_string(pos.y) << std::endl;
				opponents.insert(std::make_pair(opponentId, pos));
			}
			SendACK(ACK_NEW_CONNECTION, packetIDRecived);
		}
		else if (cmd == OK_POSITION) {
			packet >> myPlayer->position;
			if (myPlayer->resending.find(packetIDRecived) != myPlayer->resending.end())
				myPlayer->resending.erase(packetIDRecived);
	
		}
		else if (cmd == REFRESH_POSITIONS) {
			packet >> opponentId;
			if (opponents.find(opponentId) != opponents.end()) {
				Position pos;
				packet >> pos;
				opponents.find(opponentId)->second = pos;
			}
			SendACK(ACK_REFRESH_POSITIONS, packetIDRecived);

		}
		else if (cmd == DISCONNECTION) {
			packet >> opponentId;
			if (opponents.find(opponentId) != opponents.end()) {
				std::cout << "An opponent disconnected. ID: " << std::to_string(opponentId) << std::endl;
				opponents.erase(opponentId);
			}
			SendACK(ACK_DISCONNECTION, packetIDRecived);
		}

	} packet.clear();
}


void GameManager() {

	sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Traffic Game - Client: " + myPlayer->nickname);
	//c.restart();

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
			packet << TRY_POSITION << packetID << myPlayer->ID << myPlayer->MapAccumMovements.find(idMovements)->first << myPlayer->MapAccumMovements.find(idMovements)->second; //montar un paquet amb totes les posicions acumulades

			myPlayer->resending.insert(std::make_pair(packetID, packet));
			packetID++;
			idMovements++; //una avegada enviem ja puc incrementar
			clockPositions.restart; 
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
						/*
						sf::Packet packet;
						Position trypos = Position{ myPlayer->position.x - 1,myPlayer->position.y };
						packet << TRY_POSITION << packetID << myPlayer->ID << trypos; //poner packetID
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						*/

						if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
							myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ -1,0 },  Position{ myPlayer->position.x - 1,myPlayer->position.y } }));
						}
						else { //si ja existeix acumulo moviments, actualitzem
							myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += -1;
							myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += 0;
							myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += -1;
							myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += 0;
						}
					
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) //moure personatge dreta
				{
						/*
						sf::Packet packet;
						Position trypos = Position{ myPlayer->position.x + 1,myPlayer->position.y };
						packet << TRY_POSITION << packetID << myPlayer->ID << trypos; //poner packetID
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						*/
					if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ +1,0 },  Position{ myPlayer->position.x + 1,myPlayer->position.y } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += 1;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += 1;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += 0;
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) //moure personatge dalt
				{		
						/*
						sf::Packet packet;
						Position trypos = Position{ myPlayer->position.x,myPlayer->position.y - 1 };
						packet << TRY_POSITION << packetID << myPlayer->ID << trypos; //poner packetID
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						*/
					if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ 0,-1 },  Position{ myPlayer->position.x, myPlayer->position.y -1 } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += -1;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += -1;
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) //moure personatge baix
				{
						/*
						sf::Packet packet;
						Position trypos = Position{ myPlayer->position.x,myPlayer->position.y + 1 };
						packet << TRY_POSITION << packetID << myPlayer->ID << trypos; //poner packetID
						myPlayer->resending.insert(std::make_pair(packetID, packet));
						packetID++;
						*/
					if (myPlayer->MapAccumMovements.find(idMovements) != myPlayer->MapAccumMovements.end()) {  //sino exiteix, ho sigui que encara no s'ha fet cap moviment en aquest temps, el poso a la llista
						myPlayer->MapAccumMovements.insert(std::make_pair(idMovements, AccumMovements{ Position{ 0, 1 },  Position{ myPlayer->position.x, myPlayer->position.y + 1 } }));
					}
					else { //si ja existeix acumulo moviments, actualitzem
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.delta.y += 1;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.x += 0;
						myPlayer->MapAccumMovements.find(idMovements)->second.absolute.y += 1;
					}
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
				//rectBlanco.setOutlineColor(sf::Color::Green);
				//rectBlanco.setOutlineThickness(2.f);s
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

		for (std::map<int8_t, Position>::iterator it = opponents.begin(); it != opponents.end(); ++it) {
			sf::Vector2f positionOpponent(it->second.x, it->second.y);
			shapeOpponent.setPosition(positionOpponent);

			window.draw(shapeOpponent);
		}

		window.display();
	}

}


void ConnectionWithServer() {

	std::cout << "Estableciendo conexion con server... \n";
	std::cout << "Type your nickname: ";
	std::getline(std::cin, myPlayer->nickname);
	sf::Packet packet;
	packet << HELLO << packetID << myPlayer->nickname; //poner packetID
	myPlayer->resending.insert(std::make_pair(packetID, packet));
	packetID++;
	packet.clear();
}


int main()
{
	socket.bind(sf::Socket::AnyPort);
	socket.setBlocking(false);

	myPlayer = new Player();

	//initial connection
	ConnectionWithServer();

	clockPositions.restart(); //a partir daqui ja es pot acabar de moure per tant fem un reset del rellotje
	GameManager();

	opponents.clear();
	socket.unbind();
	system("exit");

	return 0;
}










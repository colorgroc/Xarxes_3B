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

#define ROWS_BOOK 4
#define COLUMNS_BOOK 2

int book[ROWS_BOOK][COLUMNS_BOOK];
sf::String mensajeBook;

enum stateGame {GAME_HASNT_STARTED,  GAME_HAS_STARTED, GAME_HAS_FINISHED } bingo;
int state = 1;

sf::TcpSocket socket;
std::vector<sf::TcpSocket*> aSock;

int puerto = 5000;

sf::Socket::Status status;
std::mutex myMutex;

std::vector<std::string> aMensajes;
sf::String mensaje;

inline bool isInteger(const std::string & s)  //https://stackoverflow.com/questions/2844817/how-do-i-check-if-a-c-string-is-an-int
{
	if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

	char * p;
	strtol(s.c_str(), &p, 10);

	return (*p == 0);
}

void shared_cout(std::string msg, int option) {

	if (msg != "") {

		if (option == RECEIVED) { 
			//cojer el commad i mostrar un texto segun lo enviado
			std::string delimiter = "_"; //s'utilitza aquest delimitador per separa commad del msg
			std::string command = msg.substr(0, msg.find(delimiter)); //command
			msg.erase(0, msg.find(delimiter) + delimiter.length()); //msg te el misatge sense el commad

			if (command == "READYTOPLAY") {
				//cambiar estat del bingo
				//mostrar per pantalla el missatge que ha començat la partida
			}
			if (command == "BINGO") {
				//mostar que el jugador ha guanyat
				//cambiar estat del bingo a acabat
			}
			else if (command == "LINE") {
				//mostar que el jugador ha fet linia
			}
			else if (command == "BOTE") {
				//mostar que el jugador el bote
			}
			else if (command == "NUMBER") {
				//mostar al jugador el nou numero
			}
			else if (command == "MESSAGE") {
				aMensajes.push_back(msg);
			}
		
		}
		if (option == CONNECTION) { aMensajes.push_back(msg); }
		else if(option == WRITED) { aMensajes.push_back("Yo: " + msg); }
	}
}

void NonBlockingChat() {

	status = socket.connect("localhost", puerto, sf::milliseconds(15.f)); //bloqueo durante un tiempo
	if (status == sf::Socket::Error)
	{
		std::cout << "No se ha podido conectar con el servidor. Reintentelo de nuevo." << std::endl;
	}
	else if (status == sf::Socket::Disconnected)
	{
		std::cout << "Servidor desconectado." << std::endl;
		state = 0;
	}
	else {
		std::string texto = "Conexion con ... " + (socket.getRemoteAddress()).toString() + ":" + std::to_string(socket.getRemotePort()) + "\n";
		std::cout << texto;
	}

	socket.setBlocking(false);

	sf::RenderWindow window;
	sf::Vector2i screenDimensions(800, 600);
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	//creacio de segona pantalla que mostra sempre la cartilla del jugador
	sf::RenderWindow windowBook;
	sf::Vector2i screenDimensionsBook(300, 300);
	windowBook.create(sf::VideoMode(screenDimensionsBook.x, screenDimensionsBook.y), "MyBook");
	mensajeBook = "";

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
	
	///////
	sf::Text bookText(mensajeBook, font, 14);
	bookText.setFillColor(sf::Color(255, 255, 255));
	bookText.setStyle(sf::Text::Bold);
	
	bookText.setString(mensajeBook);
	windowBook.draw(bookText);
	windowBook.display();
	///////

	while (window.isOpen())
	{
		sf::Event evento;

		
		//sempre escoltem, tant si ha començat el joc com si no
		if (bingo != GAME_HAS_FINISHED) {
			char buffer[100];
			size_t bytesReceived;

			status = socket.receive(buffer, 100, bytesReceived);

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
						
					
						status = socket.send(s_mensaje.c_str(), s_mensaje.length(), bSent);
						

						if (status != sf::Socket::Done)
						{
							if (status == sf::Socket::Error) {
								//std::cout << "Ha fallado el envio." << std::endl;
								shared_cout("Ha fallado el envio.", CONNECTION);
							}	
							else if (status == sf::Socket::Disconnected) {
								//std::cout << "Servidor desconectado" << std::endl;
								shared_cout("Servidor desconectado.", CONNECTION);
							}
							else if (status == sf::Socket::Partial) {
								
								while (bSent < s_mensaje.length()) {
									std::string msgRest = "";
									for (size_t i = bSent; i < s_mensaje.length(); i++) {
										msgRest = s_mensaje[i];
									}
									socket.send(msgRest.c_str(), msgRest.size(), bSent);
								}
							}
						} else shared_cout(mensaje, WRITED);

						if (mensaje == "exit") {
							bingo = GAME_HAS_FINISHED;
							socket.disconnect();
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
	std::cout << "Estableciendo conexion con server... \n";
	bingo = GAME_HASNT_STARTED;
	switch (state)
	{
	case 1:
		NonBlockingChat();
		break;
	}

	system("pause");
	return 0;
}
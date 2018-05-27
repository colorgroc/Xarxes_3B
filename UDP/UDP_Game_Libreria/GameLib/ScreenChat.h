#pragma once
#include <iostream>
#include "cScreen.h"
#include "GlobalValues.h"
#include <SFML/Graphics.hpp>
class ScreenChat : public cScreen {
private:
	sf::String mensaje;
	std::string textoAEnviar = "";
	sf::Color grey = sf::Color(169, 169, 169);
	std::vector<std::string> aMensajes;
	std::mutex myMutex;
	bool globalChat;
public:
	//ScreenChat(void);
	virtual int Run(sf::RenderWindow &App);
	void shared_cout(std::string msg, std::string nickname, int cmd);
	void ReceiveChat();
};

void ScreenChat::shared_cout(std::string msg, std::string nickname, int cmd) {
	std::lock_guard<std::mutex>guard(myMutex); //impedeix acces alhora

	if (msg != "") {
		if (cmd == GLOBAL_CHAT) {
			aMensajes.push_back("Global - " + nickname + ": " + msg);
			//else aMensajes.push_back("Global - Me: " + msg);
		}
		else if (cmd == GAME_CHAT) {
			aMensajes.push_back("Game - " + nickname + ": " + msg);
			//else aMensajes.push_back("Game - Me: " + msg);
		}
	}
}

//void ScreenChat::ReceiveChat() {
//	std::string msg = "";
//	sf::Packet packet;
//	std::string nickname = "";
//	int cmd = 0;
//
//	status = socket.receive(packet, serverIP, serverPORT);
//	if (status == sf::Socket::Done) {
//		packet >> cmd >> nickname >> msg;
//		shared_cout(msg, nickname, cmd);
//	}
//}

int ScreenChat::Run(sf::RenderWindow &App) {
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

	sf::RectangleShape globalButton(sf::Vector2f(150, 30.f));
	globalButton.setPosition(App.getSize().x - 30, 50);
	globalButton.setFillColor(grey);

	sf::Text globalText;
	globalText.setFont(font);
	globalText.setStyle(sf::Text::Italic);
	globalText.setString("GLOBAL");
	globalText.setFillColor(sf::Color::White);
	globalText.setCharacterSize(26);
	globalText.setPosition(globalButton.getPosition().x + 20, globalButton.getPosition().y - 5);

	sf::RectangleShape gameButton(sf::Vector2f(150, 30.f));
	gameButton.setPosition(App.getSize().x - 80, 50);
	gameButton.setFillColor(grey);

	sf::Text gameText;
	gameText.setFont(font);
	gameText.setStyle(sf::Text::Italic);
	gameText.setString("GLOBAL");
	gameText.setFillColor(sf::Color::White);
	gameText.setCharacterSize(26);
	gameText.setPosition(gameButton.getPosition().x + 20, gameButton.getPosition().y - 5);

	sf::Event evento;
	sf::Vector2i mousePos = sf::Mouse::getPosition(App);
	sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
	//Chat();
	while (true) {
		while (App.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				return (-1);
				break;
			case sf::Event::MouseMoved:
				//global
				if (globalButton.getGlobalBounds().contains(mousePosF))
					globalButton.setFillColor(grey);
				else globalButton.setFillColor(sf::Color::Blue);
				//game
				if (gameButton.getGlobalBounds().contains(mousePosF))
					gameButton.setFillColor(grey);
				else gameButton.setFillColor(sf::Color::Blue);

				break;
			case sf::Event::MouseButtonPressed:
				if (globalButton.getGlobalBounds().contains(mousePosF) && !globalChat) globalChat = true;
				else if (gameButton.getGlobalBounds().contains(mousePosF) && globalChat) globalChat = false;
				break;
			case sf::Event::KeyPressed:
				/*if (evento.key.code == sf::Keyboard::Escape)
				window.close();*/
				if (evento.key.code == sf::Keyboard::Return)
				{
					std::string s_mensaje;

					s_mensaje = mensaje;

					sf::Packet p;
					if (globalChat) { //ferho per packets
						p << GLOBAL_CHAT << s_mensaje;
						//shared_cout(s_mensaje, myPlayer->nickname, GLOBAL_CHAT);
					}
					else {
						p << GAME_CHAT << s_mensaje;
						//shared_cout(s_mensaje, myPlayer->nickname, GAME_CHAT);
					}

					//status = socket.send(p, serverIP, serverPORT);


					/*if (status != sf::Socket::Done)
					{
						if (status == sf::Socket::Error)
							shared_cout("Ha fallado el envio", myPlayer->nickname, false);
						if (status == sf::Socket::Disconnected)
							shared_cout("Disconnected", myPlayer->nickname, false);

					}*/

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
		if (globalChat) globalButton.setFillColor(sf::Color::Blue);
		else gameButton.setFillColor(sf::Color::Blue);
		App.draw(separator);
		App.draw(globalButton);
		App.draw(gameButton);
		App.draw(globalText);
		App.draw(gameText);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			App.draw(chattingText);
		}

		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		App.draw(text);


		App.display();
		App.clear();

	}
	return (-1);
	}
}
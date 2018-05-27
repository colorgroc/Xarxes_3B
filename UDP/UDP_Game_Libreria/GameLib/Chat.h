#pragma once
//void Chat() {
//	sf::RenderWindow windowChat;
//	sf::Vector2i screenDimensions(800, 600);
//	windowChat.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");
//
//	window.create(sf::VideoMode::getDesktopMode(), "Lobby", sf::Style::Fullscreen);
//	window.clear();
//
//	sf::Font font;
//	if (!font.loadFromFile("calibri.ttf"))
//	{
//		std::cout << "Can't load the font file" << std::endl;
//	}
//
//	mensaje = "";
//
//	sf::Text chattingText(mensaje, font, 14);
//	chattingText.setFillColor(sf::Color(255, 160, 0));
//	chattingText.setStyle(sf::Text::Bold);
//
//
//	sf::Text text(mensaje, font, 14);
//	text.setFillColor(sf::Color(0, 191, 255));
//	text.setStyle(sf::Text::Italic);
//	text.setPosition(0, 560);
//
//	sf::RectangleShape separator(sf::Vector2f(800, 5));
//	separator.setFillColor(sf::Color(255, 0, 0, 255));
//	separator.setPosition(0, 550);
//
//	sf::RectangleShape globalButton(sf::Vector2f(150, 30.f));
//	globalButton.setPosition(windowChat.getSize().x - 30, 50);
//	globalButton.setFillColor(grey);
//
//	sf::Text globalText;
//	globalText.setFont(font);
//	globalText.setStyle(sf::Text::Italic);
//	globalText.setString("GLOBAL");
//	globalText.setFillColor(sf::Color::White);
//	globalText.setCharacterSize(26);
//	globalText.setPosition(globalButton.getPosition().x + 20, globalButton.getPosition().y - 5);
//
//	sf::RectangleShape gameButton(sf::Vector2f(150, 30.f));
//	gameButton.setPosition(windowChat.getSize().x - 80, 50);
//	gameButton.setFillColor(grey);
//
//	sf::Text gameText;
//	gameText.setFont(font);
//	gameText.setStyle(sf::Text::Italic);
//	gameText.setString("GLOBAL");
//	gameText.setFillColor(sf::Color::White);
//	gameText.setCharacterSize(26);
//	gameText.setPosition(gameButton.getPosition().x + 20, gameButton.getPosition().y - 5);
//
//	//-------------- create ----------------------------
//	sf::RectangleShape createButton(sf::Vector2f(300, 50.f));
//	createButton.setPosition(window.getSize().x / 2 / 2, 200);
//	createButton.setFillColor(sf::Color::White);
//
//	sf::Text createText;
//	createText.setFont(font);
//	createText.setStyle(sf::Text::Regular);
//	createText.setString("Create Game");
//	createText.setFillColor(sf::Color::Black);
//	createText.setCharacterSize(48);
//	createText.setPosition(window.getSize().x / 2 / 2 + 20, createButton.getPosition().y - 10);
//
//	//-------------- join ----------------------------
//	sf::RectangleShape joinButton(sf::Vector2f(300, 50.f));
//	joinButton.setPosition(window.getSize().x / 2 / 2, 300);
//	joinButton.setFillColor(sf::Color::White);
//
//	sf::Text joinText;
//	joinText.setFont(font);
//	joinText.setStyle(sf::Text::Regular);
//	joinText.setString("Join Game");
//	joinText.setFillColor(sf::Color::Black);
//	joinText.setCharacterSize(48);
//	joinText.setPosition(window.getSize().x / 2 / 2 + 20, joinButton.getPosition().y - 10);
//
//	//-------------- exit ----------------------------
//	sf::RectangleShape exitButton(sf::Vector2f(200.f, 50.f));
//	exitButton.setPosition(window.getSize().x / 2 / 2, 400);
//	exitButton.setFillColor(sf::Color::Red);
//
//	sf::Text exitText;
//	exitText.setFont(font);
//	exitText.setStyle(sf::Text::Regular);
//	exitText.setString("Exit");
//	exitText.setFillColor(sf::Color::White);
//	exitText.setCharacterSize(48);
//	exitText.setPosition(window.getSize().x / 2 / 2 + 50, exitButton.getPosition().y - 10);
//
//	//-------------- back ----------------------------
//	sf::RectangleShape backButton(sf::Vector2f(200.f, 50.f));
//	backButton.setPosition(50, window.getSize().y - 100);
//	backButton.setFillColor(sf::Color::Yellow);
//
//	sf::Text backText;
//	backText.setFont(font);
//	backText.setStyle(sf::Text::Regular);
//	backText.setString("Back");
//	backText.setFillColor(sf::Color::Black);
//	backText.setCharacterSize(48);
//	backText.setPosition(50 + 50, backButton.getPosition().y - 10);
//
//	//-------------- ok ----------------------------
//	sf::RectangleShape okButton(sf::Vector2f(200.f, 50.f));
//	okButton.setPosition(window.getSize().x - 300, window.getSize().y - 100);
//	okButton.setFillColor(sf::Color::Green);
//
//	sf::Text okText;
//	okText.setFont(font);
//	okText.setStyle(sf::Text::Regular);
//	okText.setFillColor(sf::Color::Black);
//	okText.setCharacterSize(48);
//	okText.setPosition(window.getSize().x - 300 + 50, okButton.getPosition().y - 10);
//
//
//	//-------------- Create: Name ----------------------------
//	sf::Text createNameText;
//	createNameText.setFont(font);
//	createNameText.setStyle(sf::Text::Regular);
//	createNameText.setString("Name: ");
//	createNameText.setFillColor(sf::Color::White);
//	createNameText.setCharacterSize(48);
//	createNameText.setPosition(window.getSize().x / 2 / 2, 150);
//
//	sf::RectangleShape createNameButton(sf::Vector2f(200, 50.f));
//	createNameButton.setPosition(window.getSize().x / 2 / 2, 200);
//	createNameButton.setFillColor(sf::Color::White);
//
//	std::string nameInput;
//	sf::Text nameText("", font, 48);
//	nameText.setPosition(window.getSize().x / 2 / 2, 200 - 10);
//	nameText.setFillColor(sf::Color::Black);
//
//	//-------------- Create: Password ----------------------------
//	sf::Text createPassText;
//	createPassText.setFont(font);
//	createPassText.setStyle(sf::Text::Regular);
//	createPassText.setString("Password:");
//	createPassText.setFillColor(sf::Color::White);
//	createPassText.setCharacterSize(48);
//	createPassText.setPosition(window.getSize().x / 2 / 2, 250);
//
//	sf::RectangleShape createPassButton(sf::Vector2f(200, 50.f));
//	createPassButton.setPosition(window.getSize().x / 2 / 2, 300);
//	createPassButton.setFillColor(sf::Color::White);
//
//	std::string passInput;
//	sf::Text passText("", font, 48);
//	passText.setPosition(window.getSize().x / 2 / 2, 300 - 10);
//	passText.setFillColor(sf::Color::Black);
//
//
//	//-------------- Create: MaxPlayers ----------------------------
//	sf::Text createMaxText;
//	createMaxText.setFont(font);
//	createMaxText.setStyle(sf::Text::Regular);
//	createMaxText.setString("Max Players:");
//	createMaxText.setFillColor(sf::Color::White);
//	createMaxText.setCharacterSize(48);
//	createMaxText.setPosition(window.getSize().x / 2 / 2, 350);
//
//	sf::RectangleShape createMaxButton(sf::Vector2f(100, 50.f));
//	createMaxButton.setPosition(window.getSize().x / 2 / 2, 400);
//	createMaxButton.setFillColor(sf::Color::White);
//
//	std::string numInput;
//	sf::Text numText("", font, 48);
//	numText.setPosition(window.getSize().x / 2 / 2, 400 - 10);
//	numText.setFillColor(sf::Color::Black);
//
//	//-------------- Join: List ----------------------------
//
//	sf::Text nameTextButton;
//	nameTextButton.setFont(font);
//	nameTextButton.setStyle(sf::Text::Regular);
//	nameTextButton.setFillColor(sf::Color::White);
//	nameTextButton.setCharacterSize(48);
//
//	sf::Text connTextButton;
//	connTextButton.setFont(font);
//	connTextButton.setStyle(sf::Text::Regular);
//	connTextButton.setFillColor(sf::Color::White);
//	connTextButton.setCharacterSize(48);
//
//	sf::Text maxTextButton;
//	maxTextButton.setFont(font);
//	maxTextButton.setStyle(sf::Text::Regular);
//	maxTextButton.setFillColor(sf::Color::White);
//	maxTextButton.setCharacterSize(48);
//
//
//	for (std::map<int32_t, PartidaClient>::iterator it = partidas.begin(); it != partidas.end(); ++it) {
//
//		sf::RectangleShape button(sf::Vector2f(1150, 50.f));
//		button.setPosition(50, positionY);
//		button.setFillColor(sf::Color::Transparent);
//
//		vectorListaPartidas.push_back(PartidaClient{ it->second.id, it->second.name, it->second.numPlayersConnected, it->second.maxPlayers });
//		listButtons.push_back(button);
//		positionY += 50;
//	}
//
//	//------------- Join: Capçalera List -------------------
//	sf::Text capName;
//	capName.setFont(font);
//	capName.setStyle(sf::Text::Italic);
//	capName.setString("Name");
//	capName.setFillColor(sf::Color::White);
//	capName.setCharacterSize(26);
//	capName.setPosition(100, 50);
//
//	sf::Text capCon;
//	capCon.setFont(font);
//	capCon.setStyle(sf::Text::Italic);
//	capCon.setString("Connected");
//	capCon.setFillColor(sf::Color::White);
//	capCon.setCharacterSize(26);
//	capCon.setPosition(900, 50);
//
//	sf::Text capMax;
//	capMax.setFont(font);
//	capMax.setStyle(sf::Text::Italic);
//	capMax.setString("Max Players");
//	capMax.setFillColor(sf::Color::White);
//	capMax.setCharacterSize(26);
//	capMax.setPosition(capCon.getPosition().x + capCon.getLocalBounds().width + 50, 50);
//
//	//-------------- Join: Sort By Name ----------------------------
//	sf::RectangleShape sortNameButton(sf::Vector2f(150, 30.f));
//	sortNameButton.setPosition(window.getSize().x - 600, 50);
//	sortNameButton.setFillColor(sf::Color::Blue);
//
//	sf::Text sortNameText;
//	sortNameText.setFont(font);
//	sortNameText.setStyle(sf::Text::Italic);
//	sortNameText.setString("by Name");
//	sortNameText.setFillColor(sf::Color::White);
//	sortNameText.setCharacterSize(26);
//	sortNameText.setPosition(sortNameButton.getPosition().x + 20, sortNameButton.getPosition().y - 5);
//
//	//-------------- Join: Sort By Connected ----------------------------
//	sf::RectangleShape sortConButton(sf::Vector2f(150, 30.f));
//	sortConButton.setPosition(window.getSize().x - 440, 50);
//	sortConButton.setFillColor(grey);
//
//	sf::Text sortConText;
//	sortConText.setFont(font);
//	sortConText.setStyle(sf::Text::Italic);
//	sortConText.setString("by nº Con");
//	sortConText.setFillColor(sf::Color::White);
//	sortConText.setCharacterSize(26);
//	sortConText.setPosition(sortConButton.getPosition().x + 20, sortConButton.getPosition().y - 5);
//
//	//-------------- Join: Sort By Max Players ----------------------------
//	sf::RectangleShape sortMaxButton(sf::Vector2f(150, 30.f));
//	sortMaxButton.setPosition(window.getSize().x - 280, 50);
//	sortMaxButton.setFillColor(grey);
//
//	sf::Text sortMaxText;
//	sortMaxText.setFont(font);
//	sortMaxText.setStyle(sf::Text::Italic);
//	sortMaxText.setString("by nº Max");
//	sortMaxText.setFillColor(sf::Color::White);
//	sortMaxText.setCharacterSize(26);
//	//sortMaxText.setPosition(sortMaxButton.getPosition().x + 20, sortMaxButton.getPosition().y - 5);
//
//	while (windowChat.isOpen())
//	{
//		sf::Event evento;
//		sf::Vector2i mousePos = sf::Mouse::getPosition(windowChat);
//		sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
//		ReceiveChat();
//		ReceiveData();
//
//		if (c.getElapsedTime().asMilliseconds() > SENDING_PING) {
//			Resend();
//			c.restart();
//		}
//
//		while (windowChat.pollEvent(evento))
//		{
//			switch (evento.type)
//			{
//			case sf::Event::Closed:
//				windowChat.close();
//				break;
//			case sf::Event::MouseMoved:
//				//global
//				if (globalButton.getGlobalBounds().contains(mousePosF))
//					globalButton.setFillColor(grey);
//				else globalButton.setFillColor(sf::Color::Blue);
//				//game
//				if (gameButton.getGlobalBounds().contains(mousePosF))
//					gameButton.setFillColor(grey);
//				else gameButton.setFillColor(sf::Color::Blue);
//
//				if (createButton.getGlobalBounds().contains(mousePosF))
//					createButton.setFillColor(grey);
//				else createButton.setFillColor(sf::Color::White);
//				//join
//				if (joinButton.getGlobalBounds().contains(mousePosF))
//					joinButton.setFillColor(grey);
//				else joinButton.setFillColor(sf::Color::White);
//				//exit
//				if (exitButton.getGlobalBounds().contains(mousePosF))
//					exitButton.setFillColor(grey);
//				else exitButton.setFillColor(sf::Color::Red);
//				//back
//				if (backButton.getGlobalBounds().contains(mousePosF))
//					backButton.setFillColor(grey);
//				else backButton.setFillColor(sf::Color::Yellow);
//				//ok
//				if (okButton.getGlobalBounds().contains(mousePosF))
//					okButton.setFillColor(grey);
//				else okButton.setFillColor(sf::Color::Green);
//				//sortName
//				if (sortNameButton.getGlobalBounds().contains(mousePosF))
//					sortNameButton.setFillColor(sf::Color::Blue);
//				else sortNameButton.setFillColor(grey);
//				//sortConnected
//				if (sortConButton.getGlobalBounds().contains(mousePosF))
//					sortConButton.setFillColor(sf::Color::Blue);
//				else sortConButton.setFillColor(grey);
//				//sortMaxPlayers
//				if (sortMaxButton.getGlobalBounds().contains(mousePosF))
//					sortMaxButton.setFillColor(sf::Color::Blue);
//				else sortMaxButton.setFillColor(grey);
//
//				if (join) {
//					for (int8_t i = 0; i < listButtons.size(); i++) {
//						if (listButtons[i].getGlobalBounds().contains(mousePosF))
//							listButtons[i].setFillColor(grey);
//						else listButtons[i].setFillColor(sf::Color::Transparent);
//					}
//
//				}
//
//				break;
//			case sf::Event::MouseButtonPressed:
//				if (globalButton.getGlobalBounds().contains(mousePosF) && !globalChat) globalChat = true;
//				else if (gameButton.getGlobalBounds().contains(mousePosF) && globalChat) globalChat = false;
//
//				if (createButton.getGlobalBounds().contains(mousePosF) && !join && !create)
//				{
//					create = true;
//					name = true;
//					createNameButton.setFillColor(grey);
//					okText.setString("Create");
//					break;
//				}
//				else if (joinButton.getGlobalBounds().contains(mousePosF) && !join && !create)
//				{
//					join = true;
//					sortByName = sortByNameDown = true;
//					std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameDown);
//					okText.setString("Join");
//					break;
//				}
//				else if (backButton.getGlobalBounds().contains(mousePosF))
//				{
//					create = join = name = password = maxNum = writePassword = false;
//					nameInput.clear();
//					numInput.clear();
//					passInput.clear();
//					nameText.setString("");
//					numText.setString("");
//					passText.setString("");
//					break;
//				}
//				else if (okButton.getGlobalBounds().contains(mousePosF))
//				{
//					if (create) {
//						sf::Packet packet;
//						//std::string str = passText.getString();
//						int32_t max = std::stoi(numInput);
//						packet << NEW_GAME << packetID << myPlayer->ID << nameInput << passInput << max;
//						myPlayer->resending.insert(std::make_pair(packetID, packet));
//						packetID++;
//					}
//					else if (join) {
//						sf::Packet packet;
//						//std::string str = passText.getString();
//						packet << JOIN_GAME << packetID << myPlayer->ID << IDPartidaJoin << passInput;
//						myPlayer->resending.insert(std::make_pair(packetID, packet));
//						packetID++;
//					}
//
//					window.close();
//					create = join = name = password = maxNum = writePassword = false;
//					nameInput.clear();
//					numInput.clear();
//					passInput.clear();
//					nameText.setString("");
//					numText.setString("");
//					passText.setString("");
//					break;
//				}
//				else if (exitButton.getGlobalBounds().contains(mousePosF) && !create && !join)
//				{
//					window.close();
//					exitGame = disconnected = true;
//					break;
//				}
//				else if (createNameButton.getGlobalBounds().contains(mousePosF) && create)
//				{
//					name = true;
//					password = maxNum = false;
//					break;
//				}
//				else if (createPassButton.getGlobalBounds().contains(mousePosF) && create)
//				{
//					password = true;
//					name = maxNum = false;
//					break;
//				}
//				else if (createMaxButton.getGlobalBounds().contains(mousePosF) && create)
//				{
//					maxNum = true;
//					name = password = false;
//					break;
//				}
//				else if (sortNameButton.getGlobalBounds().contains(mousePosF) && join)
//				{
//					sortByName = true;
//					sortByMax = sortByConnected = false;
//					if (sortByNameDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameDown);
//						sortByNameDown = !sortByNameDown;
//					}
//					else if (!sortByNameDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByNameUp);
//						sortByNameDown = !sortByNameDown;
//					}
//					break;
//				}
//				else if (sortConButton.getGlobalBounds().contains(mousePosF) && join)
//				{
//					sortByConnected = true;
//					sortByMax = sortByName = false;
//					if (sortByConnectedDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionDown);
//						sortByConnectedDown = !sortByConnectedDown;
//					}
//					else if (!sortByConnectedDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByConnectionUp);
//						sortByConnectedDown = !sortByConnectedDown;
//					}
//					break;
//				}
//				else if (sortMaxButton.getGlobalBounds().contains(mousePosF) && join)
//				{
//					sortByMax = true;
//					sortByName = sortByConnected = false;
//					if (sortByMaxDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumDown);
//						sortByMaxDown = !sortByMaxDown;
//					}
//					else if (!sortByMaxDown) {
//						std::sort(vectorListaPartidas.begin(), vectorListaPartidas.end(), SortByMaxNumUp);
//						sortByMaxDown = !sortByMaxDown;
//					}
//					break;
//				}
//				if (join) {
//
//					for (int8_t i = 0; i < vectorListaPartidas.size(); i++) {
//						if (listButtons[i].getGlobalBounds().contains(mousePosF)) {
//							writePassword = true;//ensenyar introduccio password
//							IDPartidaJoin = vectorListaPartidas[i].id;//em guardo l'id de la partida
//							break;
//						}
//					}
//				}
//
//				break;
//			case sf::Event::KeyPressed:
//				/*if (evento.key.code == sf::Keyboard::Escape)
//				window.close();*/
//				if (evento.key.code == sf::Keyboard::Return)
//				{
//					std::string s_mensaje;
//
//					s_mensaje = mensaje;
//
//					sf::Packet p;
//					if (globalChat) { //ferho per packets
//						p << GLOBAL_CHAT << s_mensaje;
//						shared_cout(s_mensaje, myPlayer->nickname, GLOBAL_CHAT);
//					}
//					else {
//						p << GAME_CHAT << s_mensaje;
//						shared_cout(s_mensaje, myPlayer->nickname, GAME_CHAT);
//					}
//
//					status = socket.send(p, serverIP, serverPORT);
//
//
//					if (status != sf::Socket::Done)
//					{
//						if (status == sf::Socket::Error)
//							shared_cout("Ha fallado el envio", myPlayer->nickname, false);
//						if (status == sf::Socket::Disconnected)
//							shared_cout("Disconnected", myPlayer->nickname, false);
//
//					}
//
//					if (aMensajes.size() > 25)
//					{
//						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
//					}
//					mensaje = "";
//				}
//				break;
//			case sf::Event::TextEntered:
//				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
//					mensaje += (char)evento.text.unicode;
//				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
//					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
//				break;
//			}
//
//		}
//		if (globalChat) globalButton.setFillColor(sf::Color::Blue);
//		else gameButton.setFillColor(sf::Color::Blue);
//		windowChat.draw(separator);
//		windowChat.draw(globalButton);
//		windowChat.draw(gameButton);
//		windowChat.draw(globalText);
//		windowChat.draw(gameText);
//		for (size_t i = 0; i < aMensajes.size(); i++)
//		{
//			std::string chatting = aMensajes[i];
//			chattingText.setPosition(sf::Vector2f(0, 20 * i));
//			chattingText.setString(chatting);
//			windowChat.draw(chattingText);
//		}
//
//		std::string mensaje_ = mensaje + "_";
//		text.setString(mensaje_);
//		windowChat.draw(text);
//
//
//		windowChat.display();
//		windowChat.clear();
////
//	}
//}
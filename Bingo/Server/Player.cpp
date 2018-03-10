#pragma once

#include <string>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>
#include <SFML\Network.hpp>

#define INITIAL_MONEY 200
#define ROWS_BOOK 3
#define COLUMNS_BOOK 5
#define BINGO_90 89

class Player {
private:
	sf::TcpSocket* clientInfo;
	int book[ROWS_BOOK * COLUMNS_BOOK];
	bool bingo;
	int money;
public:
	
	Player(sf::TcpSocket*  _clientInfo) {
		
		clientInfo = _clientInfo;
		
		//construir la cartilla amb numeros sense repeticio
		for (int i = 0; i < ROWS_BOOK * COLUMNS_BOOK; i++) {
				book[i] = rand() % BINGO_90 + 1;
			}
		
		bingo = false;
		money = INITIAL_MONEY;
	}

	void CheckBingo() {
		//recorre tota la matriu comprovant que tots els numeros son negatius
		bool isBingo = true;
		for (int i = 0; i < ROWS_BOOK * COLUMNS_BOOK; i++) {
				if (book[i] > 0) { isBingo = false; }
		}
		if (isBingo) { bingo = true; }
	}

	int CheckLine() {
		//recorre una fila i mirar si tots els numero son negatius
		bool isLine = true;
		
		int howManyLines = 0;

		for (int i = 0; i < COLUMNS_BOOK; i++) {	
			if (book[i] > 0) { isLine = false; }
		}
		if (isLine) { howManyLines+=1; }

		isLine = true;
		for (int i = 5; i < COLUMNS_BOOK + 5; i++) {
			if (book[i] > 0) { isLine = false; }
		}
		if (isLine) { howManyLines+=1; }

		isLine = true;
		for (int i = 10; i < COLUMNS_BOOK +10; i++) {
			if (book[i] > 0) { isLine = false; }
		}
		if (isLine) { howManyLines+=1; }

		return howManyLines;
	}
	bool CheckNumber(int _numberToCheck, int _currentNumberPlaying) {
		//comprobar si a la cartilla hi ha el mateix numero
		//si hi és, actualitzem la cartilla posant el numero en negatiu
		for (int i = 0; i <  ROWS_BOOK * COLUMNS_BOOK; i++) {
		
				if (book[i] == _numberToCheck && _numberToCheck == _currentNumberPlaying) 
				{ 
					book[i] = -_numberToCheck; 
					return true; 
				}
		}
		return false;
	}

	bool getBingo() {
		return bingo;
	}

	int getMoney() {
		//retornar dels diners del jugador
		return money;

	}
	void setMoney(int _moneyToSubOrAdd) {
		money += _moneyToSubOrAdd;
	}

	sf::TcpSocket* getPlayerInfo() {
		//retornar el numero del jugador
		return clientInfo;
	}

	std::string bookReadyToSend() {
		//BOOK_
		//convertir la cartilla en un string per ja poder-la enviar
		std::string stringBook;
		for (int i = 0; i < ROWS_BOOK * COLUMNS_BOOK; i++) {

			stringBook.append(std::to_string(book[i]));
			if (i == 4 || i == 9) {
				stringBook.append("\n");
			}
			else {
				stringBook.append(" ");
			}
		
		}
		return stringBook;
	}




};
#pragma once

#include <string>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>

#define INITIAL_MONEY 200
#define ROWS_BOOK 2
#define COLUMNS_BOOK 4
#define BINGO_90 89

class Player {
private:
	int numberPlayer;
	int book[ROWS_BOOK][COLUMNS_BOOK];
	bool bingo;
	bool line;
	bool lineNumber[ROWS_BOOK];
	int money;
	int randomNumber;
public:
	
	Player(int _numberPlayer) {
		numberPlayer = _numberPlayer;
		
		//construir la cartilla amb numeros sense repeticio
		srand(time(NULL));
		std::vector<int> alreadyInsideBook;

		for (int i = 0; i <= ROWS_BOOK; i++) {
			for (int j = 0; j <= COLUMNS_BOOK; j++) {
				
				while (CheckWithoutRepetition(alreadyInsideBook, rand() % BINGO_90 + 1)) {
					//find another random number, already inside
				}
				
				alreadyInsideBook.push_back(randomNumber);
				book[i][j] = randomNumber;
			}
		}
		bingo = false;
		line = false;
		money = INITIAL_MONEY;
	}

	bool CheckWithoutRepetition(std::vector<int> _myvector, int _randomNumber) {

		bool inside = false;
		for (std::vector<int>::iterator it = _myvector.begin(); it != _myvector.end(); ++it)
		{
			if (*it == _randomNumber) {
				inside = true;
			}
		}
		if (!inside) { randomNumber = _randomNumber; }
		return inside;
	}

	bool CheckBingo() {
		//recorre tota la matriu comprovant que tots els numeros son negatius
		bool isBingo = true;
		for (int i = 0; i <= ROWS_BOOK; i++) {
			for (int j = 0; j <= COLUMNS_BOOK; j++) {
				if (book[i][j] > 0) { isBingo = false; }
			}
		}
		return isBingo;
	}
	bool* CheckLine() {
		//recorre una fila i mirar si tots els numero son negatius
		for (int i = 0; i <= ROWS_BOOK; i++) {
			bool isLine = true;
			for (int j = 0; j <= COLUMNS_BOOK; j++) {
				if (book[i][j] > 0 && !lineNumber[i]) { isLine = false; }

			}
			if (isLine) { lineNumber[i] = true; }
		}

		return lineNumber;
	}
	bool CheckNumber(int _numberToCheck) {
		//comprobar si a la cartilla hi ha el mateix numero
		//si hi és, actualitzem la cartilla posant el numero en negatiu
		for (int i = 0; i <= ROWS_BOOK; i++) {
			for (int j = 0; j <= COLUMNS_BOOK; j++) {
				if (book[i][j] == _numberToCheck) { book[i][j] = -_numberToCheck; return true; }
			}
		}
		return false;
	}

	int getMoney() {
		//retornar dels diners del jugador
		return money;

	}
	void setMoney(int _moneyToSubOrAdd) {
		money += _moneyToSubOrAdd;
	}

	int getNumberPlayer() {
		//retornar el numero del jugador
		return numberPlayer;
	}

	std::string bookReadyToSend() {
		//BOOK_
		//convertir la cartilla en un string per ja poder-la enviar
		std::string stringBook;
		for (int i = 0; i <= ROWS_BOOK; i++) {
			for (int j = 0; j <= COLUMNS_BOOK; j++) {
				stringBook.append(std::to_string(book[i][j]));
				if (j == 4) {
					stringBook.append("\n");
				}
				else {
					stringBook.append(" ");
				}
			}
		}
		return stringBook;
	}




};
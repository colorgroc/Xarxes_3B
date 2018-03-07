#pragma once

#include <string>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>

#include "Player.cpp"

class Game {

	std::vector<Player> players;
	std::vector<int> alreadyPickUp;
	int bote;
	int currentNumberToPlay;

public:
	Game() {
		bote = 0;
		currentNumberToPlay = 0;
	}

	void addNewPlayerToList(Player _player) {
		//posar a dintre el vector players el nou jugador
		players.push_back(_player);
	}

	void deletePlayerList(Player _player) {
		//agafar el numero del jugador
		//eliminar de la llista segons el numero
		//(deixar pel final, quan el joc funcioni)
		int pos = _player.getNumberPlayer();
		players.erase(players.begin + pos); //tambe s'ha de actualitzar la llista de clients
	}

	int RandomWithoutRepetiton() {
		//creació del numero que es jugara ara i retornar-lo
		srand(time(NULL));

		while (CheckWithoutRepetition(alreadyPickUp, rand() % BINGO_90 + 1)) {
			//find another random number, already inside
		}

		alreadyPickUp.push_back(currentNumberToPlay);
		return currentNumberToPlay;
	}

	bool CheckWithoutRepetition(std::vector<int> _myvector, int _randomNumber) {

		bool inside = false;
		for (std::vector<int>::iterator it = _myvector.begin(); it != _myvector.end(); ++it)
		{
			if (*it == _randomNumber) {
				inside = true;
			}
		}
		if (!inside) { currentNumberToPlay = _randomNumber; }
		return inside;
	}

	int CalculatePot(int _initalBet) {
		//per cada jugador treure la aposta inicial del seu compte
		//actualitzar bote
		for (std::vector<Player>::iterator it = players.begin(); it != players.end(); ++it) {
			bote += _initalBet;
			it->setMoney(_initalBet);
		}
	}
};
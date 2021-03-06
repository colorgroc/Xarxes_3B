#pragma once

#include <string>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>

#include "Player.cpp"

#define INITIAL_BET 100

class Game {

	std::vector<int> alreadyPickUp;
	int bote;
	int currentNumberToPlay;

public:
	//std::vector<Player> players;

	Game() {
		bote = 0;
		currentNumberToPlay = 0;
	}

	int RandomWithoutRepetiton() {
		//creaci� del numero que es jugara ara i retornar-lo
		srand(time(NULL));

		if (alreadyPickUp.size() == 90) {
			return -1;
		}

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

	void CalculatePot(Player p, int numPlayers) {
		//per cada jugador treure la aposta inicial del seu compte
		//actualitzar bote
		for (int i = 0; i < numPlayers; i++) {
			bote += INITIAL_BET;
			p.setMoney(-INITIAL_BET);
		}
	}

	int getPot() {
		return bote;
	}

	int getCurrentNumberPlaying() {
		return currentNumberToPlay;
	}
};
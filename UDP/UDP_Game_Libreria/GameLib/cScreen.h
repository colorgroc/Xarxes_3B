#pragma once
#include <stdint.h>
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
class cScreen
{
public:
	virtual int Run(sf::RenderWindow &App) = 0;
};
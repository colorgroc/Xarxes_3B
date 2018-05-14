#include "GlobalValues.h"

Position PixelToCell(int16_t _x, int16_t _y)
{
	int16_t xCell = _x / SIZE_CELL;
	int16_t yCell = _y / SIZE_CELL;
	return 	Position{ xCell, yCell };
}

Position CellToPixel(int16_t _x, int16_t _y)
{
	return Position{ _x * SIZE_CELL, _y * SIZE_CELL }; //convert to pixels
}

bool Walls::CheckCollision(AccumMovements accum) { //amb pixels
	bool correctPosition = true;

	for (std::vector<Position>::iterator it = obstaclesMap.begin(); it != obstaclesMap.end(); ++it) {

		if (accum.delta.x > 0) { //moviment dreta
			if ((it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).x && (it->y == PixelToCell(accum.absolute.x, accum.absolute.y).y)) || (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x && (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y))) {
				//+ SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}

		}
		else if (accum.delta.x < 0) { //moviment esquerra
			if ((it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x && (it->y == PixelToCell(accum.absolute.x, accum.absolute.y).y)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x))) {
				// - SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}

		if (accum.delta.y > 0) { //moviment baix
			if ((it->y == PixelToCell(accum.absolute.x , accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y + SIZE_CELL).x))) {
				// + SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}
		else if (accum.delta.y < 0) { //moviment dalt
			if ((it->y == PixelToCell(accum.absolute.x , accum.absolute.y).y && (it->x == PixelToCell(accum.absolute.x, accum.absolute.y).x)) || (it->y == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).y && (it->x == PixelToCell(accum.absolute.x + SIZE_CELL, accum.absolute.y).x))) {
				//- SIZE_CELL
			correctPosition = false;
			return correctPosition;
			}
		}
	}
	return correctPosition;
}

bool Walls::CheckCollision(Position pos) { //amb pixels
	bool correctPosition = true;

	for (std::vector<Position>::iterator it = obstaclesMap.begin(); it != obstaclesMap.end(); ++it) {

		if(pos.x == it->x && pos.y == it->y)
			correctPosition = false;
	}
	return correctPosition;
}

//std::vector<std::string> Split(std::string str, std::string del) {
//	size_t pos = 0;
//	std::string temp;
//	std::vector<std::string> vec;
//	pos = str.find(del);
//	temp = str.substr(0, pos);
//	str.erase(0, pos + del.length());
//	vec.push_back(temp);
//	vec.push_back(str);
//	return vec;
//}
//
//void GetSplit(std::string var1, std::string var2, std::string str, std::string del) {
//	std::vector<std::string> vec = Split(str, del);
//	var1 = vec.front();
//	str = vec.back();
//	vec = Split(str, del);
//	var2 = vec.front();
//}

bool SortByName(const ListButtons &a, const ListButtons &b)
{
	std::string str1 = a.name.getString();
	std::string str2 = b.name.getString();
	return str1.size() < str2.size();
}

bool SortByConnection(const ListButtons &a, const ListButtons &b)
{
	std::string str1 = a.connected.getString();
	std::string str2 = b.connected.getString();
	int8_t i1 = std::stoi(str1);
	int8_t i2 = std::stoi(str2);
	return i1 < i2;
}

bool SortByMaxNum(const ListButtons &a, const ListButtons &b)
{
	std::string str1 = a.numMax.getString();
	std::string str2 = b.numMax.getString();
	int8_t i1 = std::stoi(str1);
	int8_t i2 = std::stoi(str2);
	return i1 < i2;
}

sf::Packet& operator <<(sf::Packet& Packet, const Position& pos)
{
return Packet << pos.x << pos.y;
}

sf::Packet& operator >>(sf::Packet& Packet, Position& pos)
{
return Packet >> pos.x >> pos.y;
}

sf::Packet& operator <<(sf::Packet& Packet, const AccumMovements& accum) {
	return Packet << accum.delta << accum.absolute;
}

sf::Packet& operator >>(sf::Packet& Packet, AccumMovements& accum) {
	return Packet >> accum.delta >> accum.absolute;
}


// Buttons

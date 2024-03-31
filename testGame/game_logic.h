#pragma once

#include <vector>
#include <random>
#include <set>
#include <iostream>

namespace Minesweeper {
	enum Tile {
		UNKNOWN,
		ZERO,
		ONE,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		SEVEN,
		EIGHT,
		FLAG,
		BOMB,
		BOMB_RED,
		MARKER
	};

	struct Grid {
		int width;
		int height;
		int bombCount;

		std::vector<Tile> displayTiles;
		std::vector<Tile> realTiles;

		std::mt19937 randomEngine;

		Tile& index(std::vector<Tile>& tileVector, int x, int y);

		void fill(std::set<int>& allSpaces, std::set<int>& edgeSpaces, int startIndex);

		void open(int clickX, int clickY);
		void flag(int clickX, int clickY);
		void markerFlag(int clickX, int clickY);

		int findUnsolveable(int clickX, int clickY);

		Grid(int width, int height, int bombCount, uint32_t seed);

		void generate(int clickX, int clickY);
	};
}
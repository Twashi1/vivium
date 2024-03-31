#include "game_logic.h"

namespace Minesweeper {
	Tile& Grid::index(std::vector<Tile>& tileVector, int x, int y) {
		return tileVector[y * width + x];
	}

	Grid::Grid(int width, int height, int bombCount, uint32_t seed)
		: width(width), height(height), bombCount(bombCount),
		displayTiles(width* height), realTiles(width* height),
		randomEngine(seed)
	{}

	// Ensure click position has no surrounding mines
	void Grid::generate(int clickX, int clickY) {
		// Generate n bombs and place randomly about map
		// https://stackoverflow.com/questions/57316214/efficiently-randomly-shuffling-the-bits-of-a-sequence-of-words
		int size = width * height;
		int bombsToPlace = bombCount;

		std::vector<int> bombPositions;
		bombPositions.reserve(bombCount);

		for (int index = 0; index < size; ++index) {
			std::uniform_int_distribution<int> dist(0, size - index - 1);

			if (dist(randomEngine) < bombsToPlace)
			{
				// Check if bomb is next to click position
				int y = index / width;
				int x = index - y * width;
				bool isNextToInitialClick = false;

				for (int dy = -1; dy <= 1; dy++) {
					int lookY = y + dy;

					if (lookY < 0 || lookY >= height) continue;

					for (int dx = -1; dx <= 1; dx++) {
						int lookX = x + dx;

						if (lookX < 0 || lookX >= width) continue;

						if (lookX == clickX && lookY == clickY)
						{
							isNextToInitialClick = true;
							break;
						}
					}

					if (isNextToInitialClick) break;
				}

				if (!isNextToInitialClick) {
					realTiles[index] = Tile::BOMB;
					bombPositions.push_back(index);
					--bombsToPlace;
				}
				else {
					realTiles[index] = Tile::ZERO;
				}
			}
			else {
				realTiles[index] = Tile::ZERO;
			}
		}

		for (int bombPosition : bombPositions) {
			int y = bombPosition / width;
			int x = bombPosition - y * width;

			// Look at neighbours
			for (int dy = -1; dy <= 1; dy++) {
				int lookY = y + dy;

				if (lookY < 0 || lookY >= height) continue;

				for (int dx = -1; dx <= 1; dx++) {
					int lookX = x + dx;

					if (lookX < 0 || lookX >= width) continue;

					Tile& tile = index(realTiles, lookX, lookY);

					if (tile == Tile::BOMB) continue;
					else tile = static_cast<Tile>(tile + 1);
				}
			}
		}
	}

	void Grid::open(int clickX, int clickY)
	{
		int index = clickX + clickY * width;

		Tile displayTile = displayTiles[index];
		Tile clickedTile = realTiles[index];

		if (displayTile == Tile::UNKNOWN) {
			displayTiles[index] = realTiles[index];

			if (clickedTile == Tile::BOMB)
				std::cout << "You lost!" << std::endl;
			else if (clickedTile == Tile::ZERO) {
				for (int dy = -1; dy <= 1; dy++) {
					int lookY = clickY + dy;

					if (lookY < 0 || lookY >= height) continue;

					for (int dx = -1; dx <= 1; dx++) {
						int lookX = clickX + dx;

						if (lookX < 0 || lookX >= width) continue;

						open(lookX, lookY);
					}
				}
			}
		}
	}

	void Grid::flag(int clickX, int clickY)
	{
		int index = clickX + clickY * width;

		if (displayTiles[index] == Tile::FLAG)
			displayTiles[index] = Tile::UNKNOWN;
		else
			displayTiles[index] = Tile::FLAG;
	}

	void Grid::markerFlag(int clickX, int clickY)
	{
		int index = clickX + clickY * width;

		if (displayTiles[index] == Tile::MARKER)
			displayTiles[index] = Tile::UNKNOWN;
		else
			displayTiles[index] = Tile::MARKER;
	}
	
	int Grid::findUnsolveable(int clickX, int clickY)
	{
		/*
		TODO: algorithm

		- player clicked mine
		- find number tile in area mine clicked
			- check number tile has at least one open space about it
			- for that number tile, assign all bomb permutations
				- if that permutation satisfies all numbers which had a bomb placed next to them
				- for that permutation, propagate, by assigning every bomb permutation for all adjacent,
					edge tiles with indetermined surroundings
				- otherwise, ignore the permutation
				- if we're left with no permutations, must be misflag, fail game
				- if we're left with multiple permutations
					- inspect all knowledge of tiles gained, and see if there is a common piece of information
		*/

		// Assuming player has already revealed a mine
		std::set<int> allSpaces;
		std::set<int> edgeSpaces;

		fill(allSpaces, edgeSpaces, clickX + clickY * width);

		// TODO: this hard

		return NULL;
	}
	
	void Grid::fill(std::set<int>& allSpaces, std::set<int>& edgeSpaces, int startIndex)
	{
		if (displayTiles[startIndex] != Tile::UNKNOWN)
			if (displayTiles[startIndex] != Tile::FLAG)
			{
				edgeSpaces.insert(startIndex);

				return;
			}
		else
			allSpaces.insert(startIndex);

		int y = startIndex / width;
		int x = startIndex - y * width;

		for (int dy = -1; dy <= 1; dy++) {
			int lookY = dy + y;

			if (lookY < 0 || lookY >= height) continue;

			for (int dx = -1; dx <= 1; dx++) {
				int lookX = dx + x;

				if (lookX < 0 || lookX >= width) continue;

				int lookIndex = lookX + lookY * width;

				if (!allSpaces.contains(lookIndex) && !edgeSpaces.contains(lookIndex))
					fill(allSpaces, edgeSpaces, lookIndex);
			}
		}
	}
}
#pragma once

#include <vector>
#include <random>
#include <set>
#include <iostream>

#include "../vivium4/vivium4.h"

using namespace Vivium;

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

	bool isNumberTile(Tile tile);

	const int MAX_GRID_SIZE = 32 * 32;

	struct Grid {
		int width;
		int height;
		int bombCount;

		std::vector<std::array<int, 9>> neighbourPrecomputeIndices;

		std::vector<Tile> displayTiles;
		std::vector<Tile> realTiles;

		std::mt19937 randomEngine;

		bool isFirstOpen;

		Grid() = default;
		Grid(int width, int height, int bombCount, uint32_t seed);
	};

	struct GridRenderData {
		struct InstanceData {
			F32x2 tileTranslation;
			F32x2 spriteTranslation;
		};

		struct UniformData {
			F32x2 spriteScale;
			float tileScale;
		};

		Buffer::Layout bufferLayout;
		Shader::Handle fragment, vertex;

		DescriptorLayout::Handle descriptorLayout;

		TextureAtlas atlas;
		TextureAtlas::Index tileAtlasIndex;

		Buffer::Handle vertexStaging;
		Buffer::Handle vertexDevice;

		Buffer::Handle indexStaging;
		Buffer::Handle indexDevice;

		Buffer::Handle uniform;
		Buffer::Handle instanceStorage;

		DescriptorSet::Handle descriptorSet;
		
		Pipeline::Handle pipeline;
	};

	struct RenderState {
		GridRenderData gridRenderData;
		Uniform::PushConstant matrixPushConstants;
	
		Texture::Image spriteImage;
		Texture::Handle spritesheetTexture;
	};

	struct GameState {
		float tileSize = 32.0f;
		Grid grid;
	};

	void drop(GridRenderData& gridRender, Engine::Handle engine, Allocator::Static::Pool storage);
	void drop(RenderState& state, Engine::Handle engine, Allocator::Static::Pool storage);

	void createRenderState(RenderState& state, Allocator::Static::Pool storage, Engine::Handle engine, ResourceManager::Static::Handle resourceManager);
	void createGridRenderData(GridRenderData& grid, Texture::Handle spritesheetTexture, Allocator::Static::Pool storage, Engine::Handle engine, ResourceManager::Static::Handle resourceManager);

	void finishRenderState(RenderState& state, Allocator::Static::Pool storage, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context);
	void finishGridRenderData(GridRenderData& grid, Allocator::Static::Pool storage, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, Uniform::PushConstant pushConstants);
	
	void renderGrid(GridRenderData& gridRender, Grid& grid, Commands::Context::Handle context, Math::Perspective* perspective);
	void render(GridRenderData& gridRender, Grid& grid, Commands::Context::Handle context, Window::Handle window);

	void prepareRenderGrid(GridRenderData& gridRender, GameState& gameState, Commands::Context::Handle context);
	void prepareRender(GameState& gameState, RenderState& renderState, Commands::Context::Handle context);

	void updateGame(GameState& state);

	int index1D(Grid& grid, I32x2 clickPosition);
	I32x2 index2D(Grid& grid, int index);

	enum SolutionState {
		PLAYER_FAILED,
		COLLAPSED_PERMUTATION
	};

	void open(Grid& grid, I32x2 clickPosition);
	SolutionState checkSolutions(Grid& grid, I32x2 clickPosition);
	void toggleFlag(Grid& grid, I32x2 clickPosition);
	void toggleMarker(Grid& grid, I32x2 clickPosition);

	void generate(Grid& grid, I32x2 clickPosition);
}
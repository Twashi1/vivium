#include "game_logic.h"

namespace Minesweeper {
	int index1D(Grid& grid, I32x2 clickPosition)
	{
		return clickPosition.x + clickPosition.y * grid.width;
	}

	I32x2 index2D(Grid& grid, int index)
	{
		int y = index / grid.width;
		int x = index - y * grid.width;

		return I32x2(x, y);
	}

	Grid::Grid(int width, int height, int bombCount, uint32_t seed)
		: width(width), height(height), bombCount(bombCount),
		displayTiles(width* height), realTiles(width* height),
		randomEngine(seed), isFirstOpen(true), neighbourPrecomputeIndices(width * height)
	{
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int populateIndex = index1D(*this, I32x2(x, y));

				std::array<int, 9>& neighbours = neighbourPrecomputeIndices[populateIndex];
				int neighboursArrayIndex = 0;

				for (int dy = -1; dy <= 1; dy++) {
					int neighbourY = dy + y;

					for (int dx = -1; dx <= 1; dx++) {
						int neighbourX = dx + x;

						int neighbourIndex = INT_MAX;

						if (neighbourY >= 0 && neighbourY < height && neighbourX >= 0 && neighbourX < width)
							neighbourIndex = index1D(*this, I32x2(neighbourX, neighbourY));

						neighbours[neighboursArrayIndex++] = neighbourIndex;
					}
				}
			}
		}
	}

	void updateGame(GameState& state)
	{
		F32x2 cursorPos = Input::getCursor();

		I32x2 tileClick = (cursorPos / state.tileSize).floor();

		if (tileClick.x <= state.grid.width && tileClick.y <= state.grid.height)
		{
			if (Input::get(Input::Button::BTN_LEFT).state == Input::RELEASE) {
				Minesweeper::open(state.grid, tileClick);
			}
			else if (Input::get(Input::Button::BTN_RIGHT).state == Input::RELEASE) {
				if (Input::getModifiers() & Input::Modifier::MOD_SHIFT)
					Minesweeper::toggleMarker(state.grid, tileClick);
				else
					Minesweeper::toggleFlag(state.grid, tileClick);
			}
		}
	}

	void open(Grid& grid, I32x2 clickPosition)
	{
		if (grid.isFirstOpen) {
			grid.isFirstOpen = false;

			generate(grid, clickPosition);
		}

		int index = index1D(grid, clickPosition);

		Tile displayTile = grid.displayTiles[index];
		Tile clickedTile = grid.realTiles[index];

		if (displayTile == Tile::UNKNOWN) {
			grid.displayTiles[index] = grid.realTiles[index];

			if (clickedTile == Tile::BOMB)
				std::cout << "You lost!" << std::endl;
			else if (clickedTile == Tile::ZERO) {
				std::array<int, 9> neighbours = grid.neighbourPrecomputeIndices[index];

				for (int neighbourIndex : neighbours) {
					if (neighbourIndex == INT_MAX) continue;

					open(grid, index2D(grid, neighbourIndex));
				}
			}
		}
	}

	void toggleFlag(Grid& grid, I32x2 clickPosition)
	{
		int index = index1D(grid, clickPosition);

		if (grid.displayTiles[index] == Tile::FLAG)
			grid.displayTiles[index] = Tile::UNKNOWN;
		else
			grid.displayTiles[index] = Tile::FLAG;
	}

	void toggleMarker(Grid& grid, I32x2 clickPosition)
	{
		int index = index1D(grid, clickPosition);

		if (grid.displayTiles[index] == Tile::MARKER)
			grid.displayTiles[index] = Tile::UNKNOWN;
		else
			grid.displayTiles[index] = Tile::MARKER;
	}

	void generate(Grid& grid, I32x2 clickPosition)
	{
		// Generate n bombs and place randomly about map
		// https://stackoverflow.com/questions/57316214/efficiently-randomly-shuffling-the-bits-of-a-sequence-of-words
		int size = grid.width * grid.height;
		int bombsToPlace = grid.bombCount;

		std::vector<int> bombPositions;
		bombPositions.reserve(grid.bombCount);

		for (int index = 0; index < size; ++index) {
			std::uniform_int_distribution<int> dist(0, size - index - 1);

			if (dist(grid.randomEngine) < bombsToPlace)
			{
				// Check if bomb is next to click position
				int y = index / grid.width;
				int x = index - y * grid.width;
				bool isNextToInitialClick = false;

				std::array<int, 9> neighbours = grid.neighbourPrecomputeIndices[index];

				for (int neighbour : neighbours) {
					if (neighbour == INT_MAX) continue;

					if (neighbour == index1D(grid, clickPosition)) {
						isNextToInitialClick = true;
						break;
					}
				}

				if (!isNextToInitialClick) {
					grid.realTiles[index] = Tile::BOMB;
					bombPositions.push_back(index);
					--bombsToPlace;
				}
				else {
					grid.realTiles[index] = Tile::ZERO;
				}
			}
			else {
				grid.realTiles[index] = Tile::ZERO;
			}
		}

		for (int bombPosition : bombPositions) {
			std::array<int, 9> neighbours = grid.neighbourPrecomputeIndices[bombPosition];

			for (int neighbour : neighbours) {
				if (neighbour == INT_MAX) continue;

				Tile& tile = grid.realTiles[neighbour];

				if (tile == Tile::BOMB) continue;
				else tile = static_cast<Tile>(tile + 1);
			}
		}
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

		// fill(allSpaces, edgeSpaces, clickX + clickY * width);

		// TODO: this hard

		return NULL;
	}

	void drop(GridRenderData& gridRender, Engine::Handle engine, Allocator::Static::Pool storage)
	{
		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.vertexDevice, engine);
		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.indexDevice, engine);
		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.vertexStaging, engine);
		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.indexStaging, engine);

		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.instanceStorage, engine);
		Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, gridRender.uniform, engine);

		Shader::drop(storage, gridRender.fragment, engine);
		Shader::drop(storage, gridRender.vertex, engine);

		DescriptorLayout::drop(storage, gridRender.descriptorLayout, engine);

		Pipeline::drop(storage, gridRender.pipeline, engine);
	}

	void drop(RenderState& state, Engine::Handle engine, Allocator::Static::Pool storage)
	{
		Texture::drop(VIVIUM_RESOURCE_ALLOCATED, state.spritesheetTexture, engine);

		drop(state.gridRenderData, engine, storage);
	}

	void createRenderState(RenderState& state, Allocator::Static::Pool storage, Engine::Handle engine, ResourceManager::Static::Handle resourceManager)
	{
		state.matrixPushConstants = Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0);

		state.spriteImage = Texture::Image::fromFile("testGame/res/spritesheet.png", Texture::Format::RGBA);

		std::vector<Texture::Handle> textures = ResourceManager::Static::submit(resourceManager, std::vector<Texture::Specification>({
			state.spriteImage.specification
		}));

		state.spritesheetTexture = textures[0];

		createGridRenderData(state.gridRenderData, state.spritesheetTexture, storage, engine, resourceManager);
	}

	void createGridRenderData(GridRenderData& grid, Texture::Handle spritesheetTexture, Allocator::Static::Pool storage, Engine::Handle engine, ResourceManager::Static::Handle resourceManager)
	{
		grid.bufferLayout = Buffer::createLayout(std::vector<Shader::DataType>({
			Shader::DataType::VEC2,
			Shader::DataType::VEC2
		}));

		// Create shaders
		{
			Shader::Specification fragmentShaderSpec = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/grid.frag", "testGame/res/grid_frag.spv");
			Shader::Specification vertexShaderSpec = Shader::compile(Shader::Stage::VERTEX, "testGame/res/grid.vert", "testGame/res/grid_vert.spv");

			grid.fragment = Shader::create(storage, engine, fragmentShaderSpec);
			grid.vertex = Shader::create(storage, engine, vertexShaderSpec);
		}

		// Create descriptor
		grid.descriptorLayout = DescriptorLayout::create(storage, engine, DescriptorLayout::Specification(
			std::vector<Uniform::Binding>({
				Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
				Uniform::Binding(Shader::Stage::VERTEX,   1, Uniform::Type::UNIFORM_BUFFER),
				Uniform::Binding(Shader::Stage::VERTEX,   2, Uniform::Type::STORAGE_BUFFER)
			})
		));

		// Setup texture atlas
		grid.atlas = TextureAtlas(I32x2(256), I32x2(16));
		grid.tileAtlasIndex = TextureAtlas::Index(Tile::ZERO, grid.atlas);

		// Create buffers
		{
			std::vector<Buffer::Handle> hostBuffers = ResourceManager::Static::submit(resourceManager, MemoryType::STAGING, std::vector<Buffer::Specification>({
			Buffer::Specification(4 * grid.bufferLayout.stride, Buffer::Usage::STAGING),
			Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::STAGING),
			Buffer::Specification(sizeof(GridRenderData::UniformData), Buffer::Usage::UNIFORM),
			Buffer::Specification(MAX_GRID_SIZE * sizeof(GridRenderData::InstanceData), Buffer::Usage::STORAGE)
				}));

			grid.vertexStaging = hostBuffers[0];
			grid.indexStaging = hostBuffers[1];
			grid.uniform = hostBuffers[2];
			grid.instanceStorage = hostBuffers[3];

			std::vector<Buffer::Handle> deviceBuffers = ResourceManager::Static::submit(resourceManager, MemoryType::DEVICE, std::vector<Buffer::Specification>({
				Buffer::Specification(4 * grid.bufferLayout.stride, Buffer::Usage::VERTEX),
				Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::INDEX)
				}));

			grid.vertexDevice = deviceBuffers[0];
			grid.indexDevice = deviceBuffers[1];
		}

		// Create descriptors
		{
			std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(resourceManager, std::vector<DescriptorSet::Specification>({
				DescriptorSet::Specification(grid.descriptorLayout, std::vector<Uniform::Data>({
					Uniform::Data::fromTexture(spritesheetTexture),
					Uniform::Data::fromBuffer(grid.uniform, sizeof(F32x2), 0),
					Uniform::Data::fromBuffer(grid.instanceStorage, MAX_GRID_SIZE * sizeof(GridRenderData::InstanceData), 0)
				}))
			}));

			grid.descriptorSet = descriptorSets[0];
		}
	}

	void finishRenderState(RenderState& state, Allocator::Static::Pool storage, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context)
	{
		state.spriteImage.drop();

		finishGridRenderData(state.gridRenderData, storage, engine, window, context, state.matrixPushConstants);
	}

	void prepareRenderGrid(GridRenderData& gridRender, GameState& gameState, Commands::Context::Handle context)
	{
		uint64_t gridSize = gameState.grid.width * gameState.grid.height;
		GridRenderData::InstanceData* instanceData = new GridRenderData::InstanceData[gridSize];

		// Recalculate dynamic uniform buffer
		for (uint32_t i = 0; i < gridSize; i++) {
			Minesweeper::Tile tile = gameState.grid.displayTiles[i];

			TextureAtlas::Index data = TextureAtlas::Index(tile, gridRender.atlas);

			int y = i / gameState.grid.width;
			int x = i - y * gameState.grid.width;

			instanceData[i] = GridRenderData::InstanceData{ F32x2(x * gameState.tileSize, y * gameState.tileSize), data.translation };
		}

		// Upload instance data
		Buffer::set(gridRender.instanceStorage, 0, instanceData, sizeof(GridRenderData::InstanceData) * gridSize, 0);

		GridRenderData::UniformData uniformData;
		uniformData.spriteScale = gridRender.tileAtlasIndex.scale;
		uniformData.tileScale = gameState.tileSize;

		Buffer::set(gridRender.uniform, 0, &uniformData, sizeof(uniformData), 0);
	}

	void renderGrid(GridRenderData& gridRender, Grid& grid, Commands::Context::Handle context, Math::Perspective* perspective)
	{
		Commands::pushConstants(context, perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, gridRender.pipeline);

		Commands::bindPipeline(context, gridRender.pipeline);
		Commands::bindDescriptorSet(context, gridRender.descriptorSet, gridRender.pipeline);
		Commands::bindVertexBuffer(context, gridRender.vertexDevice);
		Commands::bindIndexBuffer(context, gridRender.indexDevice);

		Commands::drawIndexed(context, 6, grid.width * grid.height);
	}

	void render(GridRenderData& gridRender, Grid& grid, Commands::Context::Handle context, Window::Handle window)
	{
		// Recalculate perspective
		Math::Perspective perspective = Math::calculatePerspective(window, 0.0f, 0.0f, 1.0f);

		renderGrid(gridRender, grid, context, &perspective);
	}

	void prepareRender(GameState& gameState, RenderState& renderState, Commands::Context::Handle context)
	{
		prepareRenderGrid(renderState.gridRenderData, gameState, context);
	}

	void finishGridRenderData(GridRenderData& grid, Allocator::Static::Pool storage, Engine::Handle engine, Window::Handle window, Commands::Context::Handle context, Uniform::PushConstant pushConstants)
	{
		grid.pipeline = Pipeline::create(storage, engine, window, Pipeline::Specification(
			std::vector<Shader::Handle>({ grid.fragment, grid.vertex }),
			grid.bufferLayout,
			std::vector<DescriptorLayout::Handle>({ grid.descriptorLayout }),
			std::vector<Uniform::PushConstant>({ pushConstants })
		));

		uint16_t indexData[6] = { 0, 1, 2, 2, 3, 0 };

		float tileVertexData[16] = {
				0.0f, 0.0f, 0.0f, 1.0f,
				1.0f, 0.0f,	1.0f, 1.0f,
				1.0f, 1.0f, 1.0f, 0.0f,
				0.0f, 1.0f,	0.0f, 0.0f
		};

		Buffer::set(grid.vertexStaging, 0, tileVertexData, sizeof(tileVertexData), 0);
		Buffer::set(grid.indexStaging, 0, indexData, sizeof(indexData), 0);
		Buffer::set(grid.uniform, 0, &grid.tileAtlasIndex.scale, sizeof(F32x2), 0);

		Commands::Context::beginTransfer(context);

		Commands::transferBuffer(context, grid.vertexStaging, grid.vertexDevice);
		Commands::transferBuffer(context, grid.indexStaging, grid.indexDevice);

		Commands::Context::endTransfer(context, engine);
	}
}

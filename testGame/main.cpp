#include "../vivium4/vivium4.h"
#include "game_logic.h"

using namespace Vivium;

constexpr uint32_t GRID_WIDTH = 16;
constexpr uint32_t GRID_HEIGHT = 16;
constexpr uint32_t BOMB_COUNT = 45;

// TODO: lazy for now
constexpr float TILE_SIZE = 20.0f;

int main(void) {
	Minesweeper::Grid grid = Minesweeper::Grid(GRID_WIDTH, GRID_HEIGHT, BOMB_COUNT, 0);
	bool isFirstClick = true;

	Shader::Specification fragmentShaderSpec = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/grid.frag", "testGame/res/grid_frag.spv");
	Shader::Specification vertexShaderSpec = Shader::compile(Shader::Stage::VERTEX, "testGame/res/grid.vert", "testGame/res/grid_vert.spv");

	Allocator::Static::Pool storage = Allocator::Static::Pool(4096);
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	Buffer::Layout bufferLayout = Buffer::createLayout(std::vector<Shader::DataType>({
		Shader::DataType::VEC2, // Coords
		Shader::DataType::VEC2	// Texture coords
	}));

	Shader::Handle fragment = Shader::create(storage, engine, fragmentShaderSpec);
	Shader::Handle vertex = Shader::create(storage, engine, vertexShaderSpec);

	DescriptorLayout::Handle descriptorLayout = DescriptorLayout::create(storage, engine, DescriptorLayout::Specification(
		std::vector<Uniform::Binding>({ 
			Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
			Uniform::Binding(Shader::Stage::VERTEX, 1, Uniform::Type::UNIFORM_BUFFER),
			Uniform::Binding(Shader::Stage::VERTEX, 2, Uniform::Type::STORAGE_BUFFER)
		})
	));

	TextureAtlas atlas = TextureAtlas(I32x2(256), I32x2(16));
	// Doesn't matter which we reference specifically here
	TextureAtlas::Index zeroSpriteIndex = TextureAtlas::Index(Minesweeper::Tile::ZERO, atlas);
	
	Uniform::PushConstant pushConstant = Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0);

	float vertexData[16] = {
		0.0f, 0.0f,				0.0f, 1.0f,
		TILE_SIZE, 0.0f,		1.0f, 1.0f,
		TILE_SIZE, TILE_SIZE,	1.0f, 0.0f,
		0.0f, TILE_SIZE,		0.0f, 0.0f
	};

	uint16_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	struct InstanceData {
		F32x2 tileTranslation;
		F32x2 spriteTranslation;
	};

	// Submit calls
	std::vector<Buffer::Handle> stagingBuffers = ResourceManager::Static::submit(manager, MemoryType::STAGING, std::vector<Buffer::Specification>({
		Buffer::Specification(4 * bufferLayout.stride, Buffer::Usage::STAGING),
		Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::STAGING),
		Buffer::Specification(sizeof(F32x2), Buffer::Usage::UNIFORM),
		Buffer::Specification(GRID_WIDTH * GRID_HEIGHT * sizeof(InstanceData), Buffer::Usage::STORAGE)
	}));

	std::vector<Buffer::Handle> deviceBuffers = ResourceManager::Static::submit(manager, MemoryType::DEVICE, std::vector<Buffer::Specification>({
		Buffer::Specification(4 * bufferLayout.stride, Buffer::Usage::VERTEX),
		Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::INDEX)
	}));

	std::vector<uint32_t> suballocations = std::vector<uint32_t>(GRID_WIDTH * GRID_HEIGHT);
	for (uint32_t i = 0; i < suballocations.size(); i++)
		suballocations[i] = sizeof(InstanceData);

	Texture::Image spriteImage = Texture::Image::fromFile("testGame/res/spritesheet.png", Texture::Format::RGBA);

	std::vector<Texture::Handle> textures = ResourceManager::Static::submit(manager, std::vector<Texture::Specification>({
		spriteImage.specification
	}));

	std::vector<DescriptorSet::Handle> descriptorSets = ResourceManager::Static::submit(manager, std::vector<DescriptorSet::Specification>({
		DescriptorSet::Specification(descriptorLayout, std::vector<Uniform::Data>({
			Uniform::Data::fromTexture(textures[0]),
			Uniform::Data::fromBuffer(stagingBuffers[2], sizeof(F32x2), 0),
			Uniform::Data::fromBuffer(stagingBuffers[3], sizeof(InstanceData) * GRID_WIDTH * GRID_HEIGHT, 0)
		}))
	}));

	ResourceManager::Static::allocate(engine, manager);

	// Can now free after creating the resource (I'm assuming image loaded onto GPU at this point)
	spriteImage.drop();

	Pipeline::Handle pipeline = Pipeline::create(storage, engine, window, Pipeline::Specification(
		std::vector<Shader::Handle>({fragment, vertex}),
		bufferLayout,
		std::vector<DescriptorLayout::Handle>({ descriptorLayout }),
		std::vector<Uniform::PushConstant>({ pushConstant })
	));

	Buffer::set(stagingBuffers[0], vertexData, sizeof(vertexData), 0);
	Buffer::set(stagingBuffers[1], squareIndices, sizeof(squareIndices), 0);
	Buffer::set(stagingBuffers[2], &zeroSpriteIndex.scale, sizeof(F32x2), 0);

	// TODO: make these namespace functions
	context->beginTransfer();

	Commands::transferBuffer(context, stagingBuffers[0], deviceBuffers[0]);
	Commands::transferBuffer(context, stagingBuffers[1], deviceBuffers[1]);

	context->endTransfer(engine);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		F32x2 cursorPos = Input::getCursor();

		I32x2 tileClick = (cursorPos / TILE_SIZE).floor();

		if (tileClick.x <= GRID_WIDTH && tileClick.y <= GRID_HEIGHT)
		{
			if (Input::get(Input::Button::BTN_LEFT).state == Input::RELEASE) {
				if (isFirstClick) {
					grid.generate(tileClick.x, tileClick.y);

					isFirstClick = false;
				}

				grid.open(tileClick.x, tileClick.y);
			}
			else if (Input::get(Input::Button::BTN_RIGHT).state == Input::RELEASE) {
				if (Input::getModifiers() & Input::Modifier::MOD_SHIFT)
					grid.markerFlag(tileClick.x, tileClick.y);
				else
					grid.flag(tileClick.x, tileClick.y);
			}
		}

		{
			InstanceData instanceData[GRID_WIDTH * GRID_HEIGHT];

			// Recalculate dynamic uniform buffer
			for (uint32_t i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++) {
				Minesweeper::Tile tile = grid.displayTiles[i];

				TextureAtlas::Index data = TextureAtlas::Index(tile, atlas);

				int y = i / GRID_WIDTH;
				int x = i - y * GRID_WIDTH;

				instanceData[i] = InstanceData{ F32x2(x * TILE_SIZE, y * TILE_SIZE), data.translation };
			}

			// Upload instance data
			Buffer::set(stagingBuffers[3], instanceData, sizeof(instanceData), 0);

			Engine::beginRender(engine, window);

			// Recalculate perspective
			Math::Perspective perspective = Math::calculatePerspective(window, 0.0f, 0.0f, 1.0f);
			Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, pipeline);

			Commands::bindPipeline(context, pipeline);
			Commands::bindDescriptorSet(context, descriptorSets[0], pipeline);
			Commands::bindVertexBuffer(context, deviceBuffers[0]);
			Commands::bindIndexBuffer(context, deviceBuffers[1]);

			Commands::drawIndexed(context, 6, GRID_WIDTH * GRID_HEIGHT);
		}

		Engine::endRender(engine);
		
		Input::update(window);

		Engine::endFrame(engine, window);
	}

	Shader::drop(storage, engine, fragment);
	Shader::drop(storage, engine, vertex);

	DescriptorLayout::drop(storage, descriptorLayout, engine);

	for (auto handle : textures)
		Texture::drop(storage, engine, handle);
	for (auto handle : stagingBuffers)
		Buffer::drop(storage, handle, engine);
	for (auto handle : deviceBuffers)
		Buffer::drop(storage, handle, engine);

	Pipeline::drop(storage, pipeline, engine);

	ResourceManager::Static::drop(storage, engine, manager);

	Commands::Context::drop(storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);

	storage.free();
}
#include "../vivium4/vivium4.h"
#include "game_logic.h"


using namespace Vivium;

constexpr uint32_t GRID_WIDTH = 20;
constexpr uint32_t GRID_HEIGHT = 20;
constexpr uint32_t BOMB_COUNT = 90;

// TODO: lazy for now
constexpr float TILE_SIZE = 20.0f;

int main(void) {
	Minesweeper::GameState state;
	state.tileSize = TILE_SIZE;
	state.grid = Minesweeper::Grid(GRID_WIDTH, GRID_HEIGHT, BOMB_COUNT, 2);

	Font::init();

	Allocator::Static::Pool storage = Allocator::Static::Pool();
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	Text::Resource textResource;

	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	textResource.submit(100, storage, manager, engine);

	Minesweeper::RenderState renderState;
	Minesweeper::createRenderState(renderState, storage, engine, manager);

	ResourceManager::Static::allocate(engine, manager);

	textResource.create(storage, window, engine, manager);

	Minesweeper::finishRenderState(renderState, storage, engine, window, context);

	Text::TextFragmentUniformData textFragmentUniformData;
	textFragmentUniformData.r = 1.0f;
	textFragmentUniformData.g = 0.0f;
	textFragmentUniformData.b = 0.0f;

	Text::TextVertexUniformData textVertexUniformData;
	textVertexUniformData.translation = F32x2(100.0f, 100.0f);

	std::string t = "test";

	textResource.setText(engine, Text::calculateMetrics(t.data(), t.size(), textResource.font), context, t.data(), t.size(), 1.0f, Text::Alignment::LEFT);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Minesweeper::updateGame(state);

		Minesweeper::prepareRender(state, renderState, context);

		Engine::beginRender(engine, window);

		// TODO: better way of common perspective
		Minesweeper::render(renderState.gridRenderData, state.grid, context, window);
		textResource.render(textFragmentUniformData, textVertexUniformData, context, Math::calculatePerspective(window, F32x2(0.0f), 0.0f, 1.0f));

		Engine::endRender(engine);
		
		Input::update(window);

		Engine::endFrame(engine, window);
	}

	textResource.drop(storage, engine);

	Minesweeper::drop(renderState, engine, storage);

	ResourceManager::Static::drop(storage, manager, engine);

	Commands::Context::drop(storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);

	storage.free();

	Font::terminate();
}
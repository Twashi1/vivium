#include "../vivium4/vivium4.h"
#include "game_logic.h"


using namespace Vivium;

constexpr uint32_t GRID_WIDTH = 16;
constexpr uint32_t GRID_HEIGHT = 16;
constexpr uint32_t BOMB_COUNT = 45;

// TODO: lazy for now
constexpr float TILE_SIZE = 32.0f;

int main(void) {
	Minesweeper::GameState state;
	state.tileSize = TILE_SIZE;
	state.grid = Minesweeper::Grid(GRID_WIDTH, GRID_HEIGHT, BOMB_COUNT, 1);

	Allocator::Static::Pool storage = Allocator::Static::Pool(4096);
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	Minesweeper::RenderState renderState;
	Minesweeper::createRenderState(renderState, storage, engine, manager);

	ResourceManager::Static::allocate(engine, manager);

	Minesweeper::finishRenderState(renderState, storage, engine, window, context);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Minesweeper::updateGame(state);

		Minesweeper::prepareRender(state, renderState, context);

		Engine::beginRender(engine, window);

		Minesweeper::render(renderState.gridRenderData, state.grid, context, window);

		Engine::endRender(engine);
		
		Input::update(window);

		Engine::endFrame(engine, window);
	}

	Minesweeper::drop(renderState, engine, storage);

	ResourceManager::Static::drop(storage, manager, engine);

	Commands::Context::drop(storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);

	storage.free();
}
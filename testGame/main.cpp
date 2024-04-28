#include "../vivium4/vivium4.h"
#include "game_logic.h"

// TODO: better API for generating signed distance field fonts
// TODO: better convention on something like Buffer::createLayout
// TODO: better name for Batch::create (submits resources, not creating)
// TODO: better values on spreadFactor for signed distance field (too dependent on distance?)

// TODO: adapt API to be near-completely c-friendly (no member functions, keep vector, for now, but change span)
// TODO: better pointer handling with buffers, some way to pass a range pointer (maybe just using std::span)
// TODO: drawing empty text gets error

using namespace Vivium;

constexpr uint32_t GRID_WIDTH = 20;
constexpr uint32_t GRID_HEIGHT = 20;
constexpr uint32_t BOMB_COUNT = 90;

// TODO: lazy for now
constexpr float TILE_SIZE = 20.0f;

/*
void minesweeper() {
	Minesweeper::GameState state;
	state.tileSize = TILE_SIZE;
	state.grid = Minesweeper::Grid(GRID_WIDTH, GRID_HEIGHT, BOMB_COUNT, 2);

	// TODO: pretty bad
	Font::init();

	Allocator::Static::Pool storage = Allocator::Static::Pool();
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	// TODO: also bad
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
*/

void textTest() {
	Font::init();

	bool regenFont = false;

	// Compile font if it doesn't exist
	if (!std::filesystem::exists("testGame/res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("testGame/res/fonts/consola.ttf", 1024, "testGame/res/fonts/consola.sdf", 64, 1.0f);
	}

	Allocator::Static::Pool storage = Allocator::Static::Pool();
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);

	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	GUI::Visual::Button::Handle button = GUI::Visual::Button::submit(storage, manager, engine, GUI::Properties{});

	ResourceManager::Static::allocate(engine, window, manager);

	GUI::Visual::Button::setup(button, context, engine);

	std::string textValue = "hello!";

	// TODO: get font function? or pass entire text handle (better maybe...)
	Text::setText(button->text, engine, Text::calculateMetrics(textValue.c_str(), textValue.length(), button->text->font), context, textValue.c_str(), textValue.length(), 1.0f, Text::Alignment::LEFT);
	// TODO: setup button vertices and indices otherwise nothing gets drawn

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Engine::beginRender(engine, window);

		Math::Perspective perspective = Math::calculatePerspective(window, F32x2(0.0f), 0.0f, 1.0f);
		GUI::Visual::Button::render(button, context, perspective);

		Engine::endRender(engine);

		Input::update(window);

		Engine::endFrame(engine, window);
	}

	GUI::Visual::Button::drop(storage, button, engine);

	ResourceManager::Static::drop(storage, manager, engine);

	Commands::Context::drop(storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);

	storage.free();

	Font::terminate();
}

int main(void) {
	textTest();

	return NULL;
}
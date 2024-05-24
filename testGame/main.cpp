#include "../vivium4/vivium4.h"

// TODO: better values on spreadFactor for signed distance field (too dependent on distance?)
// TODO: adapt API to be near-completely c-friendly (no member functions, keep vector, for now, but change span)
// TODO: better pointer handling with buffers, some way to pass a range pointer (maybe just using std::span)

// TODO: load GUI object position data from file
// TODO: GUI scene
// TODO: GUI all objects batched together in one draw call (or 2, text requires different pipeline)
// TODO: Visuals that require more complicated rendering (sliders, pressed buttons) should just pass everything through a
//	buffer that can be indexed per instance, one big struct with lots of information, or a union with overlap data wherever possible
// TODO: generalised serialisation?

// TODO: allocate vulkan resources ourselves to avoid double pointer indirection
//	+ access to vector operations

// TODO: multi-window applications (both drawing to multiple windows, and switchign between target)

// TODO: built-in shader/text loading system?

// TODO: "file" format, for passing around various files, without need for validation at each step

using namespace Vivium;

int main(void) {
	Font::init();

	bool regenFont = false;

	// Compile font if it doesn't exist
	if (!std::filesystem::exists("testGame/res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("testGame/res/fonts/consola.ttf", 512, "testGame/res/fonts/consola.sdf", 48, 1.0f);
	}

	Allocator::Static::Pool storage = Allocator::Static::Pool();
	Window::Handle window = Window::create(&storage, Window::Options{});
	Engine::Handle engine = Engine::create(&storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(&storage, engine);

	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(&storage);

	GUI::Visual::Context::Handle guiContext = GUI::Visual::Context::submit(&storage, manager, engine, window);

	GUI::Visual::Button::Handle button = GUI::Visual::Button::submit(&storage, manager, guiContext, engine, window);

	// TODO: reverse argument order
	ResourceManager::Static::allocate(engine, manager);

	// TODO: seems like "setup" is the convention
	GUI::Visual::Context::clean(guiContext, engine);

	GUI::Visual::Button::setup(button, context, engine);
	GUI::Visual::Button::setText(button, engine, window, context, "Hello world\nHi\n\nSpace");
	GUI::Object::properties(button).position = F32x2(0.0f, 0.0f);
	GUI::Object::properties(button).dimensions = F32x2(0.4f, 0.3f);
	GUI::Object::properties(button).scaleType = GUI::ScaleType::RELATIVE;
	GUI::Object::properties(button).positionType = GUI::PositionType::RELATIVE;

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Engine::beginRender(engine, window);
		Math::Perspective perspective = Math::orthogonalPerspective2D(window, F32x2(0.0f), 0.0f, 1.0f);

		GUI::Visual::Button::render(button, context, guiContext, window, perspective);

		Engine::endRender(engine);

		Input::update(window);

		Engine::endFrame(engine, window);
	}

	GUI::Visual::Context::drop(&storage, manager, guiContext, engine);
	GUI::Visual::Button::drop(&storage, manager, button, engine);

	ResourceManager::Static::drop(&storage, manager, engine);

	Commands::Context::drop(&storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(&storage, window, engine);
	Engine::drop(&storage, engine, window);

	storage.free();

	Font::terminate();

	return NULL;
}
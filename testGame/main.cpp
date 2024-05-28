#include "../vivium4/vivium4.h"

using namespace Vivium;

struct SkyPushConstants {
	F32x2 worldPosition;
};

struct State {
	ResourceManager::Static::Handle manager;
	Commands::Context::Handle context;
	Engine::Handle engine;
	Window::Handle window;
	GUI::Visual::Context::Handle guiContext;
	
	Allocator::Static::Pool storage;

	struct {
		Pipeline::Handle pipeline;
		Batch::Handle batch;
		Batch::Result batchResult;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;
	} skyGraphics;
};

void _submitSky(State& state)
{
	state.skyGraphics.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::Specification(4, 6, Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 }))));
	state.skyGraphics.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/sky.frag", "testGame/res/sky_frag.spv"));
	state.skyGraphics.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "testGame/res/sky.vert", "testGame/res/sky_vert.spv"));

	std::vector<Pipeline::PromisedHandle> pipelines = ResourceManager::Static::submit(state.manager,
		std::vector<Pipeline::Specification>({
			Pipeline::Specification::fromWindow(
				std::vector<Shader::Handle>({ state.skyGraphics.fragmentShader, state.skyGraphics.vertexShader }),
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({}),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::FRAGMENT, sizeof(SkyPushConstants), 0)}),
				state.engine,
				state.window
			)
		}));

	state.skyGraphics.pipeline = pipelines[0];
}

void _setupSky(State& state) {
	Batch::submitRectangle(state.skyGraphics.batch, 0, -1.0f, -1.0f, 1.0f, 1.0f);
	Batch::submitRectangle(state.skyGraphics.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.skyGraphics.batch, 4, std::vector<uint16_t>({ 0, 3, 2, 2, 1, 0 }));
	state.skyGraphics.batchResult = Batch::endSubmission(state.skyGraphics.batch, state.context, state.engine);

	Shader::drop(VIVIUM_NULL_ALLOCATOR, state.skyGraphics.fragmentShader, state.engine);
	Shader::drop(VIVIUM_NULL_ALLOCATOR, state.skyGraphics.vertexShader, state.engine);
}

void _freeSky(State& state) {
	Batch::drop(&state.storage, state.skyGraphics.batch, state.engine);

	Pipeline::drop(VIVIUM_NULL_ALLOCATOR, state.skyGraphics.pipeline, state.engine);
}

void _renderSky(State& state) {
	SkyPushConstants pushConstants;
	pushConstants.worldPosition = F32x2(0.0f);

	Commands::bindPipeline(state.context, state.skyGraphics.pipeline);
	Commands::bindVertexBuffer(state.context, state.skyGraphics.batchResult.vertexBuffer);
	Commands::bindIndexBuffer(state.context, state.skyGraphics.batchResult.indexBuffer);
	Commands::pushConstants(state.context, &pushConstants, sizeof(SkyPushConstants), 0, Shader::Stage::FRAGMENT, state.skyGraphics.pipeline);
	Commands::drawIndexed(state.context, state.skyGraphics.batchResult.indexCount, 1);
}

void initialise(State& state) {
	state.storage = Allocator::Static::Pool{};
	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);
	state.context = Commands::Context::create(&state.storage, state.engine);
	state.manager = ResourceManager::Static::create(&state.storage);
	
	Input::init(state.window);

	// state.guiContext = GUI::Visual::Context::submit(&state.storage, state.manager, state.engine, state.window);
	_submitSky(state);

	// TODO: reverse argument order
	ResourceManager::Static::allocate(state.engine, state.manager);

	// GUI::Visual::Context::clean(state.guiContext, state.engine);
	_setupSky(state);
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);
		Commands::Context::flush(state.context, state.engine);

		Engine::beginRender(state.engine, state.window);

		_renderSky(state);

		Engine::endRender(state.engine);

		Input::update(state.window);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	_freeSky(state);

	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	// GUI::Visual::Context::drop(&state.storage, state.guiContext, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);
	// Delete order not obvious
	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	state.storage.free();
}

int main(void) {
	Font::init();

	bool regenFont = false;

	// Compile font if it doesn't exist
	if (!std::filesystem::exists("testGame/res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("testGame/res/fonts/consola.ttf", 512, "testGame/res/fonts/consola.sdf", 48, 1.0f);
	}

	State state;

	initialise(state);
	gameloop(state);
	terminate(state);

	Font::terminate();

	return NULL;
}
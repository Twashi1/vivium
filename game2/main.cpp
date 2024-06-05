#include "../vivium4/vivium4.h"

using namespace Vivium;

struct CharacterUniform {
	Color color;
	float scale;
	F32x2 position;
	float time;
};

struct SkyPushConstants {
	F32x2 worldPosition;
	F32x2 screenDimensions;
	float time;
};

struct GroundUniform {
	float time;
};

struct State {
	Engine::Handle engine;
	Window::Handle window;
	Commands::Context::Handle context;
	ResourceManager::Static::Handle manager;
	
	Storage::Static::Pool storage;

	Math::Perspective perspective;

	Math::Polygon box;

	struct {
		Pipeline::Handle pipeline;
		DescriptorSet::Handle descriptorSet;
		DescriptorLayout::Handle descriptorLayout;

		Buffer::Handle uniformBuffer;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;

		Batch::Handle batch;

		Time::Timer timer;

		Physics::Body body;
	} character;

	struct {
		Pipeline::Handle pipeline;
		Batch::Handle batch;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;
	} sky;

	struct {
		Pipeline::Handle pipeline;
		Batch::Handle batch;

		Buffer::Handle uniformBuffer;

		DescriptorSet::Handle descriptorSet;
		DescriptorLayout::Handle descriptorLayout;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;
	} ground;
};

const uint64_t maxGroundVertices = 16;

// Character
void _submitCharacter(State& state) {
	state.character.body.shape = Physics::Shape(&state.box);
	state.character.body.position = F32x2(200.0f, 100.0f);
	state.character.body.velocity = F32x2(0.0f);
	state.character.body.force = F32x2(0.0f);
	state.character.body.material = Physics::Material::Default;
	state.character.body.inverseInertia = 1.0f / state.box.inertia();
	state.character.body.inverseMass = 1.0f / state.box.area();

	state.character.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::Specification(4, 6, Buffer::Layout::fromTypes(
		std::vector<Shader::DataType>{
			Shader::DataType::VEC2,
			Shader::DataType::VEC2
		}
	)));

	state.character.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../game2/res/player.frag", "../../../game2/res/player_frag.spv"));
	state.character.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../game2/res/player.vert", "../../../game2/res/player_vert.spv"));

	ResourceManager::Static::submit(state.manager, &state.character.uniformBuffer, MemoryType::UNIFORM, std::vector<Buffer::Specification>({
		Buffer::Specification(sizeof(CharacterUniform), Buffer::Usage::UNIFORM)
	}));

	state.character.descriptorLayout = DescriptorLayout::create(&state.storage, state.engine, DescriptorLayout::Specification(
		std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, 0, Uniform::Type::UNIFORM_BUFFER) })
	));

	ResourceManager::Static::submit(state.manager, &state.character.descriptorSet, std::vector<DescriptorSet::Specification>({
		DescriptorSet::Specification(state.character.descriptorLayout, std::vector<Uniform::Data>({
			Uniform::Data::fromBuffer(state.character.uniformBuffer, sizeof(CharacterUniform), 0)
		}))
	}));

	ResourceManager::Static::submit(state.manager, &state.character.pipeline, std::vector<Pipeline::Specification>({
		Pipeline::Specification::fromWindow(
			std::vector<Shader::Handle>({ state.character.fragmentShader, state.character.vertexShader }),
			Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
			std::vector<DescriptorLayout::Handle>({ state.character.descriptorLayout }),
			std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0) }),
			state.engine,
			state.window
		)
	}));
}

void _submitGround(State& state) {
	state.ground.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::Specification(maxGroundVertices * 4, maxGroundVertices * 6, Buffer::Layout::fromTypes(
		std::vector<Shader::DataType>{
			Shader::DataType::VEC2,
			Shader::DataType::VEC2
		}
	)));

	state.ground.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../game2/res/ground.frag", "../../../game2/res/ground_frag.spv"));
	state.ground.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../game2/res/ground.vert", "../../../game2/res/ground_vert.spv"));

	ResourceManager::Static::submit(state.manager, &state.ground.uniformBuffer, MemoryType::UNIFORM, std::vector<Buffer::Specification>({
		Buffer::Specification(sizeof(GroundUniform), Buffer::Usage::UNIFORM)
		}));

	state.ground.descriptorLayout = DescriptorLayout::create(&state.storage, state.engine, DescriptorLayout::Specification(
		std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, 0, Uniform::Type::UNIFORM_BUFFER) })
	));

	ResourceManager::Static::submit(state.manager, &state.ground.descriptorSet, std::vector<DescriptorSet::Specification>({
		DescriptorSet::Specification(state.character.descriptorLayout, std::vector<Uniform::Data>({
			Uniform::Data::fromBuffer(state.ground.uniformBuffer, sizeof(GroundUniform), 0)
		}))
	}));

	ResourceManager::Static::submit(state.manager, &state.ground.pipeline, std::vector<Pipeline::Specification>({
		Pipeline::Specification::fromWindow(
			std::vector<Shader::Handle>({ state.ground.fragmentShader, state.ground.vertexShader }),
			Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
			std::vector<DescriptorLayout::Handle>({ state.ground.descriptorLayout }),
			std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0) }),
			state.engine,
			state.window
		)
	}));
}

void _submitSky(State& state) {
	state.sky.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::Specification(4, 6, Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 }))));
	state.sky.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../game2/res/sky.frag", "../../../game2/res/sky_frag.spv"));
	state.sky.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../game2/res/sky.vert", "../../../game2/res/sky_vert.spv"));

	ResourceManager::Static::submit(state.manager,
		&state.sky.pipeline,
		std::vector<Pipeline::Specification>({
			Pipeline::Specification::fromWindow(
				std::vector<Shader::Handle>({ state.sky.fragmentShader, state.sky.vertexShader }),
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({}),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, sizeof(SkyPushConstants), 0)}),
				state.engine,
				state.window
			)
			}));
}

void _setupCharacter(State& state) {
	Batch::submitRectangle(state.character.batch, 0, -0.5f, -0.5f, 0.5f, 0.5f);
	Batch::submitRectangle(state.character.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.character.batch, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));
	Batch::endSubmission(state.character.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.character.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.character.vertexShader, state.engine);
}

void _setupSky(State& state) {
	Batch::submitRectangle(state.sky.batch, 0, -1.0f, -1.0f, 1.0f, 1.0f);
	Batch::submitRectangle(state.sky.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.sky.batch, 4, std::vector<uint16_t>({ 0, 3, 2, 2, 1, 0 }));
	Batch::endSubmission(state.sky.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.sky.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.sky.vertexShader, state.engine);
}

void _setupGround(State& state) {
	Batch::submitRectangle(state.ground.batch, 0, 0.0f, -200.0f, 400.0f, -100.0f);
	Batch::submitRectangle(state.ground.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.ground.batch, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));
	Batch::endSubmission(state.ground.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.ground.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.ground.vertexShader, state.engine);
}

void _freeCharacter(State& state) {
	Buffer::drop(VIVIUM_NULL_STORAGE, state.character.uniformBuffer, state.engine);
	Batch::drop(&state.storage, state.character.batch, state.engine);
	DescriptorSet::drop(VIVIUM_NULL_STORAGE, state.character.descriptorSet);
	DescriptorLayout::drop(&state.storage, state.character.descriptorLayout, state.engine);
	Pipeline::drop(VIVIUM_NULL_STORAGE, state.character.pipeline, state.engine);
}

void _freeSky(State& state) {
	Batch::drop(&state.storage, state.sky.batch, state.engine);

	Pipeline::drop(VIVIUM_NULL_STORAGE, state.sky.pipeline, state.engine);
}

void _freeGround(State& state) {
	Buffer::drop(VIVIUM_NULL_STORAGE, state.ground.uniformBuffer, state.engine);
	Batch::drop(&state.storage, state.ground.batch, state.engine);
	DescriptorSet::drop(VIVIUM_NULL_STORAGE, state.ground.descriptorSet);
	DescriptorLayout::drop(&state.storage, state.ground.descriptorLayout, state.engine);
	Pipeline::drop(VIVIUM_NULL_STORAGE, state.ground.pipeline, state.engine);
}

void _updateCharacter(State& state) {
	F32x2 heading = F32x2(0.0f);

	if (Input::get(Input::KEY_D)) {
		heading.x += 1.0f;
	}
	if (Input::get(Input::KEY_A)) {
		heading.x -= 1.0f;
	}

	state.character.body.velocity = 10.0f * heading;

	Physics::update(state.character.body, 1.0f);
}

void _renderCharacter(State& state) {
	CharacterUniform characterUniform;
	characterUniform.color = Color(0.05f, 0.0f, 0.1f);
	characterUniform.scale = 80.0f;
	characterUniform.time = state.character.timer.getTime();
	characterUniform.position = state.character.body.position;

	Buffer::set(state.character.uniformBuffer, 0, &characterUniform, sizeof(CharacterUniform));

	Commands::bindPipeline(state.context, state.character.pipeline);
	Commands::bindDescriptorSet(state.context, state.character.descriptorSet, state.character.pipeline);
	Commands::bindVertexBuffer(state.context, Batch::vertexBuffer(state.character.batch));
	Commands::bindIndexBuffer(state.context, Batch::indexBuffer(state.character.batch));
	Commands::pushConstants(state.context, &state.perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, state.character.pipeline);
	
	Commands::drawIndexed(state.context, 6, 1);
}

void _renderGround(State& state) {
	GroundUniform groundUniform;
	groundUniform.time = state.character.timer.getTime();

	Buffer::set(state.ground.uniformBuffer, 0, &groundUniform, sizeof(GroundUniform));

	Commands::bindPipeline(state.context, state.ground.pipeline);
	Commands::bindDescriptorSet(state.context, state.ground.descriptorSet, state.ground.pipeline);
	Commands::bindVertexBuffer(state.context, Batch::vertexBuffer(state.ground.batch));
	Commands::bindIndexBuffer(state.context, Batch::indexBuffer(state.ground.batch));
	Commands::pushConstants(state.context, &state.perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, state.ground.pipeline);

	Commands::drawIndexed(state.context, Batch::indexCount(state.ground.batch), 1);
}

void _renderSky(State& state) {
	SkyPushConstants pushConstants;
	pushConstants.worldPosition = F32x2(state.character.body.position.x, -state.character.body.position.y) - F32x2(Window::dimensions(state.window)) * 0.5f;
	pushConstants.time = state.character.timer.getTime();
	pushConstants.screenDimensions = Window::dimensions(state.window);

	Commands::bindPipeline(state.context, state.sky.pipeline);
	Commands::bindVertexBuffer(state.context, Batch::vertexBuffer(state.sky.batch));
	Commands::bindIndexBuffer(state.context, Batch::indexBuffer(state.sky.batch));
	Commands::pushConstants(state.context, &pushConstants, sizeof(SkyPushConstants), 0, Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, state.sky.pipeline);
	Commands::drawIndexed(state.context, Batch::indexCount(state.sky.batch), 1);
}

void initialise(State& state) {
	Font::init();

	state.box = Math::Polygon::fromBox(F32x2(64.0f));

	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);

	Input::init(state.window);

	state.context = Commands::Context::create(&state.storage, state.engine);

	state.manager = ResourceManager::Static::create(&state.storage);

	_submitCharacter(state);
	_submitSky(state);
	_submitGround(state);

	ResourceManager::Static::allocate(state.engine, state.manager);

	_setupCharacter(state);
	_setupSky(state);
	_setupGround(state);
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);
		Commands::Context::flush(state.context, state.engine);

		Input::update(state.window);

		_updateCharacter(state);

		Engine::beginRender(state.engine, state.window);

		// TODO
		state.perspective = Math::orthogonalPerspective2D(state.window, F32x2(state.character.body.position.x, state.character.body.position.y) - F32x2(Window::dimensions(state.window)) * 0.5f, 0.0f, 1.0f);
		_renderSky(state);
		_renderCharacter(state);
		_renderGround(state);

		Engine::endRender(state.engine);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	_freeCharacter(state);
	_freeSky(state);
	_freeGround(state);

	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);

	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	Font::terminate();
}

int main(void) {
	State state;

	initialise(state);
	gameloop(state);
	terminate(state);
}
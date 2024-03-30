#include "../vivium4/vivium4.h"

using namespace Vivium;

int main(void) {
	Shader::Specification fragmentShaderSpec = Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/tri.frag", "testGame/res/tri_frag.spv");
	Shader::Specification vertexShaderSpec = Shader::compile(Shader::Stage::VERTEX, "testGame/res/tri.vert", "testGame/res/tri_vert.spv");

	Allocator::Static::Pool storage = Allocator::Static::Pool(4096);
	Window::Handle window = Window::create(storage, Window::Options{});
	Engine::Handle engine = Engine::create(storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(storage, engine);
	
	// TODO: create function for this
	ResourceManager::Static::Handle manager = ResourceManager::Static::create(storage);

	Buffer::Layout bufferLayout = Buffer::createLayout(std::vector<Shader::DataType>({
		Shader::DataType::VEC2
	}));

	Shader::Handle fragment = Shader::create(storage, engine, fragmentShaderSpec);
	Shader::Handle vertex = Shader::create(storage, engine, vertexShaderSpec);

	Math::Perspective perspective = Math::calculatePerspective(window, 0.0f, 0.0f, 1.0f);
	
	Uniform::PushConstant pushConstant = Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0);

	float vertexData[8] = {
		0.0f, 0.0f,
		100.0f, 0.0f,
		100.0f, 100.0f,
		0.0f, 100.0f
	};

	uint16_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

	// Submit calls
	std::vector<Buffer::Handle> stagingBuffers = ResourceManager::Static::submit(manager, MemoryType::STAGING, std::vector<Buffer::Specification>({
		Buffer::Specification(4 * sizeof(F32x2), Buffer::Usage::STAGING),
		Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::STAGING)
	}));

	std::vector<Buffer::Handle> deviceBuffers = ResourceManager::Static::submit(manager, MemoryType::DEVICE, std::vector<Buffer::Specification>({
		Buffer::Specification(4 * sizeof(F32x2), Buffer::Usage::VERTEX),
		Buffer::Specification(6 * sizeof(uint16_t), Buffer::Usage::INDEX)
	}));

	ResourceManager::Static::allocate(engine, manager);

	Pipeline::Handle pipeline = Pipeline::create(storage, engine, window, Pipeline::Specification(
		std::vector<Shader::Handle>({fragment, vertex}),
		bufferLayout,
		std::vector<DescriptorLayout::Handle>({}),
		std::vector<Uniform::PushConstant>({ pushConstant })
	));

	Buffer::set(stagingBuffers[0], vertexData, sizeof(vertexData), 0);
	Buffer::set(stagingBuffers[1], squareIndices, sizeof(squareIndices), 0);

	context->beginTransfer();

	Commands::transferBuffer(context, stagingBuffers[0], deviceBuffers[0]);
	Commands::transferBuffer(context, stagingBuffers[1], deviceBuffers[1]);

	context->endTransfer(engine);

	while (Window::isOpen(window, engine)) {
		Engine::beginFrame(engine, window);
		Commands::Context::flush(context, engine);

		Commands::pushConstants(context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, pipeline);

		Engine::beginRender(engine, window);

		Commands::bindVertexBuffer(context, deviceBuffers[0]);
		Commands::bindIndexBuffer(context, deviceBuffers[1]);
		Commands::bindPipeline(context, pipeline);

		Commands::drawIndexed(context, 6, 1);

		// DO stuff

		Engine::endRender(engine);
		Engine::endFrame(engine, window);
	}

	Shader::drop(storage, engine, fragment);
	Shader::drop(storage, engine, vertex);

	for (auto handle : stagingBuffers)
		handle->drop(engine);
	for (auto handle : deviceBuffers)
		handle->drop(engine);

	// TODO: move
	pipeline->drop(engine);

	ResourceManager::Static::drop(storage, engine, manager);

	// TODO: delete order not obvious
	Commands::Context::drop(storage, context, engine);
	Window::drop(storage, window, engine);
	Engine::drop(storage, engine, window);
	storage.free();
}
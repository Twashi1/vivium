#include "../vivium4/vivium4.h"

// TODO: better values on spreadFactor for signed distance field (too dependent on distance?)

// TODO: adapt API to be near-completely c-friendly (no member functions, keep vector, for now, but change span)
// TODO: better pointer handling with buffers, some way to pass a range pointer (maybe just using std::span)

// TODO: load GUI object position data from file
// TODO: no separation of GUI visual and GUI object, GUI visual should be simple to use
// TODO: GUI scene
// TODO: GUI all objects batched together in one draw call (or 2, text requires different pipeline)
// TODO: Visuals that require more complicated rendering (sliders, pressed buttons) should just pass everything through a
//	buffer that can be indexed per instance, one big struct with lots of information, or a union with overlap data wherever possible
// TODO: generalised serialisation?

// TODO: allocate vulkan resources ourselves to avoid double pointer indirection
//	+ access to vector operations

// TODO: multi-window applications (both drawing to multiple windows, and switchign between target)
// TODO: OpenGL-style framebuffers

// TODO: built-in shader/text loading system?

// TODO: "file" format, for passing around various files, without need for validation at each step

using namespace Vivium;

int main(void) {
	Font::init();

	bool regenFont = false;

	// Compile font if it doesn't exist
	if (!std::filesystem::exists("testGame/res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("testGame/res/fonts/consola.ttf", 1024, "testGame/res/fonts/consola.sdf", 64, 1.0f);
	}

	Allocator::Static::Pool storage = Allocator::Static::Pool();
	Window::Handle window = Window::create(&storage, Window::Options{});
	Engine::Handle engine = Engine::create(&storage, Engine::Options{}, window);
	Commands::Context::Handle context = Commands::Context::create(&storage, engine);

	Input::init(window);

	ResourceManager::Static::Handle manager = ResourceManager::Static::create(&storage);

	GUI::Visual::Context::Handle guiContext = GUI::Visual::Context::submit(&storage, manager, engine, window);

	Batch::Handle batch = Batch::submit(&storage, engine, manager, Batch::Specification(4, 6, Buffer::Layout::fromTypes(
		std::vector<Shader::DataType>({Shader::DataType::VEC2, Shader::DataType::VEC2})
	)));

	Batch::Handle batch2 = Batch::submit(&storage, engine, manager, Batch::Specification(4, 6, Buffer::Layout::fromTypes(
		std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })
	)));

	DescriptorLayout::Handle descriptorLayout = DescriptorLayout::create(&storage, engine, DescriptorLayout::Specification(
		std::vector<Uniform::Binding>({
			Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::UNIFORM_BUFFER)
		})
	));

	DescriptorLayout::Handle descriptorLayout2 = DescriptorLayout::create(&storage, engine, DescriptorLayout::Specification(
		std::vector<Uniform::Binding>({
			Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::FRAMEBUFFER)
		})
	));

	std::vector<Buffer::PromisedHandle> buffers = ResourceManager::Static::submit(
		manager,
		MemoryType::UNIFORM,
		std::vector<Buffer::Specification>({Buffer::Specification(sizeof(float) * 3, Buffer::Usage::UNIFORM)})
	);

	Shader::Handle shaders[2] = {
		Shader::create(&storage, engine, Shader::compile(Shader::Stage::VERTEX, "testGame/res/tri.vert", "testGame/res/tri_vert.spv")),
		Shader::create(&storage, engine, Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/tri.frag", "testGame/res/tri_frag.spv"))
	};

	Shader::Handle shaders2[2] = {
		Shader::create(&storage, engine, Shader::compile(Shader::Stage::VERTEX, "testGame/res/invert.vert", "testGame/res/invert_vert.spv")),
		Shader::create(&storage, engine, Shader::compile(Shader::Stage::FRAGMENT, "testGame/res/invert.frag", "testGame/res/invert_frag.spv"))
	};

	std::vector<Framebuffer::Handle> framebuffers = ResourceManager::Static::submit(manager, std::vector<Framebuffer::Specification>({
		Framebuffer::Specification(Window::dimensions(window), Texture::Format::RGBA, 1)
	}));

	std::vector<DescriptorSet::Handle> descriptors = ResourceManager::Static::submit(
		manager,
		std::vector<DescriptorSet::Specification>({
			DescriptorSet::Specification(descriptorLayout, std::vector<Uniform::Data>({
				Uniform::Data::fromBuffer(buffers[0], sizeof(float) * 3, 0)
			})),
			DescriptorSet::Specification(descriptorLayout2, std::vector<Uniform::Data>({
				Uniform::Data::fromFramebuffer(framebuffers[0])
			}))
		})
	);

	GUI::Visual::Button::Handle button = GUI::Visual::Button::submit(&storage, manager, guiContext, engine, window);

	std::vector<Pipeline::PromisedHandle> pipelines = ResourceManager::Static::submit(manager,
		std::vector<Pipeline::Specification>({
			Pipeline::Specification::fromFramebuffer(
				{ shaders, 2 },
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({ descriptorLayout }),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0) }),
				framebuffers[0],
				VK_SAMPLE_COUNT_1_BIT
			),
			Pipeline::Specification::fromWindow(
				{ shaders2, 2 },
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({ descriptorLayout2 }),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0) }),
				engine,
				window
			)
		})
	);

	// TODO: reverse argument order
	ResourceManager::Static::allocate(engine, manager);

	// TODO: seems like "setup" is the convention
	GUI::Visual::Context::clean(guiContext, engine);

	GUI::Visual::Button::setup(button, context, engine);
	GUI::Visual::Button::setText(button, engine, window, context, "Hello world\nHi");
	GUI::Object::properties(button).position = F32x2(0.0f, 0.0f);
	GUI::Object::properties(button).dimensions = F32x2(0.4f, 0.3f);
	GUI::Object::properties(button).scaleType = GUI::ScaleType::RELATIVE;
	GUI::Object::properties(button).positionType = GUI::PositionType::RELATIVE;

	Shader::drop(&storage, shaders[0], engine);
	Shader::drop(&storage, shaders[1], engine);
	Shader::drop(&storage, shaders2[0], engine);
	Shader::drop(&storage, shaders2[1], engine);

	F32x2 windowDim = Window::dimensions(window);
	Batch::submitRectangle(batch, 0, 0.0f, 0.0f, windowDim.x, windowDim.y);
	Batch::submitRectangle(batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(batch, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));

	Batch::submitRectangle(batch2, 0, 0.0f, 0.0f, 200.0f, 200.0f);
	Batch::submitRectangle(batch2, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(batch2, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));

	Batch::Result result = Batch::endSubmission(batch, context, engine);
	Batch::Result result2 = Batch::endSubmission(batch2, context, engine);

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

	GUI::Visual::Context::drop(&storage, guiContext, engine);
	GUI::Visual::Button::drop(&storage, button, engine);

	Framebuffer::drop(VIVIUM_RESOURCE_ALLOCATED, framebuffers[0], engine);

	Batch::drop(&storage, batch, engine);
	Batch::drop(&storage, batch2, engine);

	Buffer::drop(VIVIUM_RESOURCE_ALLOCATED, buffers[0], engine);

	DescriptorLayout::drop(&storage, descriptorLayout, engine);
	DescriptorLayout::drop(&storage, descriptorLayout2, engine);

	Pipeline::drop(VIVIUM_RESOURCE_ALLOCATED, pipelines[0], engine);
	Pipeline::drop(VIVIUM_RESOURCE_ALLOCATED, pipelines[1], engine);

	ResourceManager::Static::drop(&storage, manager, engine);

	Commands::Context::drop(&storage, context, engine);
	// TODO: delete order not obvious, needs to be window before engine
	Window::drop(&storage, window, engine);
	Engine::drop(&storage, engine, window);

	storage.free();

	Font::terminate();

	return NULL;
}
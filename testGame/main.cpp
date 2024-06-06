#include "../vivium4/vivium4.h"

#ifdef ENABLED
using namespace Vivium;

struct SkyPushConstants {
	F32x2 worldPosition;
	F32x2 screenDimensions;
	float time;
};

struct CharacterUniformData {
	F32x2 translation;
	F32x2 scale;
	float angle;
};

struct SpriteUniformData {
	F32x2 textureTranslation;
	F32x2 textureScale;
	F32x2 spriteTranslation;
	F32x2 spriteScale;
	float spriteAngle;
	// Get us up to 48 bytes
	int _fill0, _fill1, _fill2;
};

enum EnemyType {
	BACTERIA,
	TRUCK
};

struct EnemyData {
	EnemyType type;
	float hitpoints;
	Time::Timer knockbackTimer;

	Physics::Body body;
};

struct Projectile {
	F32x2 position;
	F32x2 velocity;
	float angle;
	float damage;
};

const uint64_t SPRITE_LIMIT = 256;

struct State {
	ResourceManager::Static::Handle manager;
	Commands::Context::Handle context;
	Engine::Handle engine;
	Window::Handle window;
	GUI::Visual::Context::Handle guiContext;
	
	Storage::Static::Pool storage;

	Texture::Handle textureAtlas;

	Time::Timer timer;

	Font::Font font;

	std::vector<EnemyData> enemies;
	std::vector<Projectile> projectiles;
	Time::Timer projectileTimer;

	uint64_t enemiesSpawned = 0;

	Math::Polygon enemyPolygon;

	struct {
		Pipeline::Handle pipeline;
		DescriptorLayout::Handle descriptorLayout;
		DescriptorSet::Handle descriptorSet;

		Buffer::Handle uniformBuffer;
		Batch::Handle batch;
		Batch::Result batchResult;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;

		F32x2 position;
		float angle;
	} character;

	struct {
		GUI::Visual::Text::Handle text;
		GUI::Visual::Text::Metrics textMetrics;
	} score;

	struct {
		Pipeline::Handle pipeline;
		DescriptorLayout::Handle descriptorLayout;
		DescriptorSet::Handle descriptorSet;

		Buffer::Handle storageBuffer;
		Batch::Handle batch;
		Batch::Result batchResult;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;

		std::vector<SpriteUniformData> sprites;
	} spriteRenderer;

	struct {
		Pipeline::Handle pipeline;
		Batch::Handle batch;
		Batch::Result batchResult;

		Shader::Handle fragmentShader;
		Shader::Handle vertexShader;
	} skyGraphics;
};

void _submitCharacter(State& state) {
	state.character.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::PipelineSpecification(4, 6, Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 }))));
	state.character.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../testGame/res/character.frag", "../../../testGame/res/character_frag.spv"));
	state.character.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../testGame/res/character.vert", "../../../testGame/res/character_vert.spv"));

	ResourceManager::Static::submit(state.manager, &state.character.uniformBuffer, MemoryType::UNIFORM, std::vector<Buffer::PipelineSpecification>({ Buffer::PipelineSpecification(sizeof(CharacterUniformData), Buffer::Usage::UNIFORM) }));

	state.character.descriptorLayout = DescriptorLayout::create(&state.storage, state.engine, DescriptorLayout::PipelineSpecification(std::vector<Uniform::Binding>({ Uniform::Binding(Shader::Stage::VERTEX, 0, Uniform::Type::UNIFORM_BUFFER), Uniform::Binding(Shader::Stage::FRAGMENT, 1, Uniform::Type::TEXTURE) })));

	ResourceManager::Static::submit(state.manager, &state.character.descriptorSet, std::vector<DescriptorSet::PipelineSpecification>({
		DescriptorSet::PipelineSpecification(state.character.descriptorLayout, std::vector<Uniform::Data>({
			Uniform::Data::fromBuffer(state.character.uniformBuffer, sizeof(CharacterUniformData), 0),
			Uniform::Data::fromTexture(state.textureAtlas)
		}))
	}));

	ResourceManager::Static::submit(state.manager,
		&state.character.pipeline,
		std::vector<Pipeline::PipelineSpecification>({
			Pipeline::PipelineSpecification::fromWindow(
				std::vector<Shader::Handle>({ state.character.fragmentShader, state.character.vertexShader }),
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({ state.character.descriptorLayout }),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
				state.engine,
				state.window
			)
			}));

	state.character.position = F32x2(0.0f);
}

void _submitSpriteRenderer(State& state) {
	state.spriteRenderer.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::PipelineSpecification(4, 6, Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 }))));
	state.spriteRenderer.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../testGame/res/creature.frag", "../../../testGame/res/creature_frag.spv"));
	state.spriteRenderer.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../testGame/res/creature.vert", "../../../testGame/res/creature_vert.spv"));

	ResourceManager::Static::submit(state.manager, &state.spriteRenderer.storageBuffer, MemoryType::UNIFORM, std::vector<Buffer::PipelineSpecification>({ Buffer::PipelineSpecification(SPRITE_LIMIT * sizeof(SpriteUniformData), Buffer::Usage::STORAGE) }));

	state.spriteRenderer.descriptorLayout = DescriptorLayout::create(
		&state.storage,
		state.engine,
		DescriptorLayout::PipelineSpecification(std::vector<Uniform::Binding>({
			Uniform::Binding(Shader::Stage::FRAGMENT, 0, Uniform::Type::TEXTURE),
			Uniform::Binding(Shader::Stage::VERTEX, 1, Uniform::Type::STORAGE_BUFFER),
		})));

	ResourceManager::Static::submit(state.manager,
		&state.spriteRenderer.descriptorSet,
		std::vector<DescriptorSet::PipelineSpecification>({
		DescriptorSet::PipelineSpecification(state.spriteRenderer.descriptorLayout, std::vector<Uniform::Data>({
			Uniform::Data::fromTexture(state.textureAtlas),
			Uniform::Data::fromBuffer(state.spriteRenderer.storageBuffer, SPRITE_LIMIT * sizeof(SpriteUniformData), 0)
		}))
	}));

	ResourceManager::Static::submit(state.manager,
		&state.spriteRenderer.pipeline,
		std::vector<Pipeline::PipelineSpecification>({
			Pipeline::PipelineSpecification::fromWindow(
				std::vector<Shader::Handle>({ state.spriteRenderer.fragmentShader, state.spriteRenderer.vertexShader }),
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({ state.spriteRenderer.descriptorLayout }),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::VERTEX, sizeof(Math::Perspective), 0)}),
				state.engine,
				state.window
			)
			}));
}

void _submitSky(State& state)
{
	state.skyGraphics.batch = Batch::submit(&state.storage, state.engine, state.manager, Batch::PipelineSpecification(4, 6, Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 }))));
	state.skyGraphics.fragmentShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::FRAGMENT, "../../../testGame/res/sky.frag", "../../../testGame/res/sky_frag.spv"));
	state.skyGraphics.vertexShader = Shader::create(&state.storage, state.engine, Shader::compile(Shader::Stage::VERTEX, "../../../testGame/res/sky.vert", "../../../testGame/res/sky_vert.spv"));

	ResourceManager::Static::submit(state.manager,
		&state.skyGraphics.pipeline,
		std::vector<Pipeline::PipelineSpecification>({
			Pipeline::PipelineSpecification::fromWindow(
				std::vector<Shader::Handle>({ state.skyGraphics.fragmentShader, state.skyGraphics.vertexShader }),
				Buffer::Layout::fromTypes(std::vector<Shader::DataType>({ Shader::DataType::VEC2, Shader::DataType::VEC2 })),
				std::vector<DescriptorLayout::Handle>({}),
				std::vector<Uniform::PushConstant>({ Uniform::PushConstant(Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, sizeof(SkyPushConstants), 0)}),
				state.engine,
				state.window
			)
		}));
}

void _submitScore(State& state) {
	state.score.text = GUI::Visual::Text::submit(&state.storage, state.manager, state.engine, state.guiContext, GUI::Visual::Text::PipelineSpecification(64, state.font));
	state.score.text->base->properties = GUI::Properties(
		F32x2(0.0f),
		F32x2(0.0f),
		GUI::PositionType::RELATIVE, GUI::ScaleType::FIXED,
		GUI::Anchor::LEFT, GUI::Anchor::BOTTOM, GUI::Anchor::LEFT, GUI::Anchor::BOTTOM
	);
}

void _setupCharacter(State& state) {
	Batch::submitRectangle(state.character.batch, 0, -0.5f, -0.5f, 0.5f, 0.5f);
	// TODO: should really be turned into a calculation, not storing state
	Math::AtlasIndex index = Math::textureAtlasIndex(I32x2(256), I32x2(32), I32x2(0));
	Batch::submitRectangle(state.character.batch, 1, index.left, index.bottom, index.right, index.top);
	Batch::endShape(state.character.batch, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));
	state.character.batchResult = Batch::endSubmission(state.character.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.character.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.character.vertexShader, state.engine);

	state.character.position = F32x2(0.0f);
}

void _setupSky(State& state) {
	Batch::submitRectangle(state.skyGraphics.batch, 0, -1.0f, -1.0f, 1.0f, 1.0f);
	Batch::submitRectangle(state.skyGraphics.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.skyGraphics.batch, 4, std::vector<uint16_t>({ 0, 3, 2, 2, 1, 0 }));
	state.skyGraphics.batchResult = Batch::endSubmission(state.skyGraphics.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.skyGraphics.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.skyGraphics.vertexShader, state.engine);
}

void _setupSpriteRenderer(State& state) {
	Batch::submitRectangle(state.spriteRenderer.batch, 0, -0.5f, -0.5f, 0.5f, 0.5f);
	Batch::submitRectangle(state.spriteRenderer.batch, 1, 0.0f, 0.0f, 1.0f, 1.0f);
	Batch::endShape(state.spriteRenderer.batch, 4, std::vector<uint16_t>({ 0, 1, 2, 2, 3, 0 }));
	state.spriteRenderer.batchResult = Batch::endSubmission(state.spriteRenderer.batch, state.context, state.engine);

	Shader::drop(&state.storage, state.spriteRenderer.fragmentShader, state.engine);
	Shader::drop(&state.storage, state.spriteRenderer.vertexShader, state.engine);
}

void _updateScore(State& state) {
	GUI::Object::update(state.score.text, Window::dimensions(state.window));

	std::string text = std::format("Score: {}", state.enemiesSpawned);
	state.score.textMetrics = GUI::Visual::Text::calculateMetrics(text, state.font);

	state.score.text->base->properties.dimensions = F32x2(state.score.textMetrics.maxLineWidth, state.score.textMetrics.totalHeight) * 0.5f;
	state.score.text->base->properties.position = F32x2(30.0f, 30.0f);

	GUI::Visual::Text::setText(state.score.text, state.engine, state.score.textMetrics, state.context, text, GUI::Visual::Text::Alignment::LEFT);
}

void _freeSky(State& state) {
	Batch::drop(&state.storage, state.skyGraphics.batch, state.engine);

	Pipeline::drop(VIVIUM_NULL_STORAGE, state.skyGraphics.pipeline, state.engine);
}

void _freeCharacter(State& state) {
	Batch::drop(&state.storage, state.character.batch, state.engine);
	DescriptorLayout::drop(&state.storage, state.character.descriptorLayout, state.engine);
	DescriptorSet::drop(VIVIUM_NULL_STORAGE, state.character.descriptorSet);
	Buffer::drop(VIVIUM_NULL_STORAGE, state.character.uniformBuffer, state.engine);
	Pipeline::drop(VIVIUM_NULL_STORAGE, state.character.pipeline, state.engine);
}

void _freeSpriteRenderer(State& state) {
	Batch::drop(&state.storage, state.spriteRenderer.batch, state.engine);
	DescriptorLayout::drop(&state.storage, state.spriteRenderer.descriptorLayout, state.engine);
	DescriptorSet::drop(VIVIUM_NULL_STORAGE, state.spriteRenderer.descriptorSet);
	Buffer::drop(VIVIUM_NULL_STORAGE, state.spriteRenderer.storageBuffer, state.engine);
	Pipeline::drop(VIVIUM_NULL_STORAGE, state.spriteRenderer.pipeline, state.engine);
}

void _freeScore(State& state) {
	GUI::Visual::Text::drop(&state.storage, state.score.text, state.engine);
}

void _renderSky(State& state) {
	SkyPushConstants pushConstants;
	pushConstants.worldPosition = F32x2(state.character.position.x, -state.character.position.y) - F32x2(Window::dimensions(state.window)) * 0.5f;
	pushConstants.time = state.timer.getTime();
	pushConstants.screenDimensions = Window::dimensions(state.window);

	Commands::bindPipeline(state.context, state.skyGraphics.pipeline);
	Commands::bindVertexBuffer(state.context, state.skyGraphics.batchResult.vertexBuffer);
	Commands::bindIndexBuffer(state.context, state.skyGraphics.batchResult.indexBuffer);
	Commands::pushConstants(state.context, &pushConstants, sizeof(SkyPushConstants), 0, Shader::Stage::FRAGMENT | Shader::Stage::VERTEX, state.skyGraphics.pipeline);
	Commands::drawIndexed(state.context, state.skyGraphics.batchResult.indexCount, 1);
}

void _renderCharacter(State& state) {
	Math::Perspective perspective = Math::orthogonalPerspective2D(state.window, state.character.position - F32x2(Window::dimensions(state.window)) * 0.5f, 0.0f, 1.0f);

	CharacterUniformData characterData;
	characterData.scale = F32x2(64.0f);
	characterData.translation = state.character.position;
	characterData.angle = state.character.angle;

	Commands::bindPipeline(state.context, state.character.pipeline);
	Commands::bindDescriptorSet(state.context, state.character.descriptorSet, state.character.pipeline);
	Commands::bindVertexBuffer(state.context, state.character.batchResult.vertexBuffer);
	Commands::bindIndexBuffer(state.context, state.character.batchResult.indexBuffer);
	Commands::pushConstants(state.context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, state.character.pipeline);
	Buffer::set(state.character.uniformBuffer, 0, &characterData, sizeof(CharacterUniformData));

	Commands::drawIndexed(state.context, state.character.batchResult.indexCount, 1);
}

void _renderSprite(State& state) {
	if (state.spriteRenderer.sprites.size() == 0) return;
	if (state.spriteRenderer.sprites.size() > SPRITE_LIMIT) {
		VIVIUM_LOG(Log::FATAL, "Exceeded sprite limit: {}", state.spriteRenderer.sprites.size());
	}

	// TODO: common perspective
	Math::Perspective perspective = Math::orthogonalPerspective2D(state.window, state.character.position - F32x2(Window::dimensions(state.window)) * 0.5f, 0.0f, 1.0f);
	Buffer::set(state.spriteRenderer.storageBuffer, 0, state.spriteRenderer.sprites.data(), state.spriteRenderer.sprites.size() * sizeof(SpriteUniformData));

	Commands::bindPipeline(state.context, state.spriteRenderer.pipeline);
	Commands::bindDescriptorSet(state.context, state.spriteRenderer.descriptorSet, state.spriteRenderer.pipeline);
	Commands::bindVertexBuffer(state.context, state.spriteRenderer.batchResult.vertexBuffer);
	Commands::bindIndexBuffer(state.context, state.spriteRenderer.batchResult.indexBuffer);
	Commands::pushConstants(state.context, &perspective, sizeof(Math::Perspective), 0, Shader::Stage::VERTEX, state.spriteRenderer.pipeline);

	Commands::drawIndexed(state.context, state.spriteRenderer.batchResult.indexCount, state.spriteRenderer.sprites.size());

	state.spriteRenderer.sprites = {};
}

void _renderScore(State& state) {
	GUI::Visual::Text::render(state.score.text, state.score.textMetrics, state.context, state.guiContext, Color::Black, F32x2(0.5f), Math::orthogonalPerspective2D(state.window, F32x2(0.0f), 0.0f, 1.0f));
}

void _updateCharacter(State& state) {
	constexpr float pi = 3.1415926f;
	const float moveSpeed = 10.0f;

	F32x2 heading = F32x2(0.0f);
	
	if (Input::get(Input::KEY_W)) {
		heading.y += moveSpeed;
	}
	if (Input::get(Input::KEY_S)) {
		heading.y -= moveSpeed;
	}
	if (Input::get(Input::KEY_A)) {
		heading.x -= moveSpeed;
	}
	if (Input::get(Input::KEY_D)) {
		heading.x += moveSpeed;
	}

	if (Input::get(Input::KEY_SPACE) && state.projectileTimer.getTime() > 0.5f) {
		state.projectileTimer.reset();

		if (state.projectiles.size() < 100)
			state.projectiles.push_back(Projectile(state.character.position, F32x2(cos(state.character.angle + 0.5f * pi), sin(state.character.angle + 0.5f * pi)) * F32x2(20.0f), state.character.angle, 1.0f));
	}

	if (heading == F32x2(0.0f)) return;

	state.character.angle = atan2f(heading.y, heading.x) - 0.5f * pi;
	state.character.position += heading;
}

void _updateEnemies(State& state) {
	Math::AtlasIndex indices[2] = { Math::textureAtlasIndex(I32x2(256), I32x2(32), I32x2(1, 0)), Math::textureAtlasIndex(I32x2(256), I32x2(32), I32x2(3, 0)) };

	constexpr float pi = 3.1415926f;
	// TODO: rename
	const float moveSpeed = 200.0f;
	const float maxSpeed = 5.0f;

	const float kickbackForce = 16000.0f;

	std::vector<uint8_t> projectileBitmask(state.projectiles.size());
	for (uint64_t i = 0; i < projectileBitmask.size(); i++) {
		projectileBitmask[i] = 1;
	}

	std::vector<uint8_t> enemyBitmask(state.enemies.size());
	for (uint64_t i = 0; i < enemyBitmask.size(); i++) {
		enemyBitmask[i] = 1;
	}

	for (uint64_t j = 0; j < state.enemies.size(); j++) {
		EnemyData& enemy = state.enemies[j];
		Math::AtlasIndex index = indices[static_cast<int>(enemy.type)];

		state.spriteRenderer.sprites.push_back(SpriteUniformData(
			index.translation,
			index.scale,
			enemy.body.position,
			F32x2(32.0f),
			enemy.body.angle
		));

		if (enemy.knockbackTimer.getTime() > 0.15f) {
			F32x2 heading = state.character.position - enemy.body.position;
			float lengthSquared = F32x2::dot(heading, heading);
			heading = F32x2::normalise(heading);

			if (abs(lengthSquared) < 1.0f) heading = F32x2(0.0f);

			enemy.body.force = heading * moveSpeed;
			if (F32x2::length(enemy.body.velocity) > 1.0f)
				enemy.body.velocity = F32x2::normalise(enemy.body.velocity) * std::min(maxSpeed, F32x2::length(enemy.body.velocity));
			enemy.body.angle = atan2f(enemy.body.velocity.y, enemy.body.velocity.x);

			// TODO: make sure projectiles arent too far (kill if they are)
			for (uint64_t i = 0; i < state.projectiles.size(); i++) {
				Projectile& projectile = state.projectiles[i];

				if (F32x2::length(enemy.body.position - projectile.position) < 20.0f) {
					enemy.body.force = F32x2::normalise(projectile.velocity) * kickbackForce;
					projectileBitmask[i] = 0;
					enemy.knockbackTimer.reset();
					enemy.hitpoints -= projectile.damage;
				}

				if (enemy.hitpoints == 0.0f) {
					enemyBitmask[j] = 0;
				}
			}
		}
	}

	std::vector<Projectile> remaining;
	remaining.reserve(projectileBitmask.size());

	for (uint64_t i = 0; i < projectileBitmask.size(); i++) {
		if (projectileBitmask[i]) {
			remaining.push_back(state.projectiles[i]);
		}
	}

	state.projectiles = std::move(remaining);

	std::vector<EnemyData> remainingEnemies;
	remainingEnemies.reserve(enemyBitmask.size());

	for (uint64_t i = 0; i < enemyBitmask.size(); i++) {
		if (enemyBitmask[i]) {
			remainingEnemies.push_back(state.enemies[i]);
		}
	}

	state.enemies = std::move(remainingEnemies);

	Math::AtlasIndex index2 = Math::textureAtlasIndex(I32x2(256), I32x2(16), I32x2(4, 0));

	for (Projectile& projectile : state.projectiles) {
		state.spriteRenderer.sprites.push_back(SpriteUniformData(
			index2.translation,
			index2.scale,
			projectile.position,
			F32x2(16.0f),
			projectile.angle
		));
	}
}

void initialise(State& state) {
	state.storage = Storage::Static::Pool{};
	state.window = Window::create(&state.storage, Window::Options{});
	state.engine = Engine::create(&state.storage, Engine::Options{}, state.window);
	state.context = Commands::Context::create(&state.storage, state.engine);
	state.manager = ResourceManager::Static::create(&state.storage);

	state.enemyPolygon = Math::Polygon::fromBox(F32x2(32.0f));

	bool regenFont = true;

	// TODO: change to vivium4/res/
	// Compile font if it doesn't exist
	if (!std::filesystem::exists("testGame/res/fonts/consola.sdf") || regenFont)
	{
		Font::compileSignedDistanceField("testGame/res/fonts/consola.ttf", 512, "testGame/res/fonts/consola.sdf", 48, 1.0f);
	}

	state.font = Font::Font::fromDistanceFieldFile("testGame/res/fonts/consola.sdf");
	
	Input::init(state.window);

	state.guiContext = GUI::Visual::Context::submit(&state.storage, state.manager, state.engine, state.window);
	_submitSky(state);

	ResourceManager::Static::submit(state.manager, &state.textureAtlas, std::vector<Texture::PipelineSpecification>({ Texture::PipelineSpecification::fromImageFile("../../../testGame/res/game.png", Texture::Format::RGBA, Texture::Filter::NEAREST) }));
	
	_submitCharacter(state);
	_submitScore(state);
	_submitSpriteRenderer(state);

	// TODO: reverse argument order
	ResourceManager::Static::allocate(state.engine, state.manager);

	GUI::Visual::Context::setup(state.guiContext, state.context, state.engine);
	_setupSky(state);
	_setupCharacter(state);
	_setupSpriteRenderer(state);
}

void _spawnEnemies(State& state) {
	
	const float radius = 256.0f;
	const uint64_t maxEnemies = 10;
	const float angleDifference = 2.0f * 3.14159f / maxEnemies;

	if (state.enemies.size() < maxEnemies) {
		Physics::Body body;
		body.position = state.character.position + radius * F32x2(cos(angleDifference * state.enemiesSpawned), sin(angleDifference * state.enemiesSpawned));
		++state.enemiesSpawned;

		body.velocity = F32x2(0.0f);
		body.force = F32x2(0.0f);
		body.angle = 0.0f;
		body.angularVelocity = 0.0f;
		body.torque = 0.0f;
		body.material = Physics::Material::Default;
		body.inverseMass = 1.0f / state.enemyPolygon.area();
		body.inverseInertia = 1.0f / state.enemyPolygon.inertia();
		body.shape = Physics::Shape(&state.enemyPolygon);
		body.enabled = true;

		state.enemies.push_back(EnemyData(
			state.enemies.size() % 2 ? EnemyType::BACTERIA : EnemyType::TRUCK,
			2.0f,
			Time::Timer(),
			body
		));
	}
}

void _updatePhysics(State& state) {
	std::vector<Physics::Body*> enemies;
	
	for (EnemyData& enemy : state.enemies) {
		enemies.push_back(&enemy.body);
	}

	for (Projectile& projectile : state.projectiles) {
		projectile.position += projectile.velocity;
	}
	
	Physics::solve(enemies, enemies);
	
	for (Physics::Body* body : enemies) {
		Physics::update(*body, 1.0f);
	}
}

void gameloop(State& state) {
	while (Window::isOpen(state.window, state.engine)) {
		Engine::beginFrame(state.engine, state.window);

		_spawnEnemies(state);

		Commands::Context::flush(state.context, state.engine);

		_updateCharacter(state);
		_updateEnemies(state);
		_updatePhysics(state);
		_updateScore(state);

		Engine::beginRender(state.engine, state.window);

		_renderSky(state);
		_renderSprite(state);
		_renderCharacter(state);
		_renderScore(state);

		Engine::endRender(state.engine);

		Input::update(state.window);

		Engine::endFrame(state.engine, state.window);
	}
}

void terminate(State& state) {
	_freeSky(state);
	_freeCharacter(state);
	_freeSpriteRenderer(state);
	_freeScore(state);

	Texture::drop(VIVIUM_NULL_STORAGE, state.textureAtlas, state.engine);

	GUI::Visual::Context::drop(&state.storage, state.guiContext, state.engine);
	Commands::Context::drop(&state.storage, state.context, state.engine);
	
	ResourceManager::Static::drop(&state.storage, state.manager, state.engine);
	// Delete order not obvious
	Window::drop(&state.storage, state.window, state.engine);
	Engine::drop(&state.storage, state.engine, state.window);

	state.storage.free();
}

int testGame(void) {
	Font::init();

	State state;

	initialise(state);
	gameloop(state);
	terminate(state);

	Font::terminate();

	return NULL;
}
#endif
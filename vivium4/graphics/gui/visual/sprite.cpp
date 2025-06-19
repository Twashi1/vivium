#include "sprite.h"

namespace Vivium {
	Sprite createSprite(GUIContext& guiContext, SpriteSpecification specification)
	{
		Sprite sprite{};

		sprite.base = createGUIElement(guiContext, GUIElementType::DEFAULT);
		sprite.textureOffset = specification.textureOffset;
		sprite.textureScale = specification.textureScale;

		addChild(specification.parent, { &sprite.base, 1 }, guiContext);

		return sprite;
	}

	void renderSprites(const std::span<Sprite*> sprites, CommandContext& context, GUIContext& guiContext, Window& window)
	{
		std::vector<_GUISpriteInstanceData> spriteData(sprites.size());

		for (uint64_t i = 0; i < sprites.size(); i++) {
			Sprite& sprite = *sprites[i];

			_GUISpriteInstanceData instance;
			instance.position = properties(sprite.base, guiContext).truePosition;
			instance.scale = properties(sprite.base, guiContext).trueDimensions;
			instance.texturePosition = sprite.textureOffset;
			instance.textureScale = sprite.textureScale;
			
			spriteData[i] = instance;
		}

		Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		setBuffer(guiContext.sprite.storageBuffer.resource, 0, spriteData.data(), spriteData.size() * sizeof(_GUISpriteInstanceData));
		cmdBindPipeline(context, guiContext.sprite.pipeline.resource);
		cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		cmdBindDescriptorSet(context, guiContext.sprite.descriptorSet.resource, guiContext.sprite.pipeline.resource);
		cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.sprite.pipeline.resource);
		cmdDrawIndexed(context, 6, spriteData.size());
	}
}
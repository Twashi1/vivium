#pragma once

#include "context.h"
#include "../../color.h"
#include "../../../math/atlas.h"

namespace Vivium {
	// Want to be able to draw sprites quickly and efficiently
	//	likely easiest that all sprites are in one atlas
	//  can stitch atlas at runtime

	struct Sprite {
		GUIElementReference base;
		F32x2 textureScale;
		F32x2 textureOffset;
	};

	struct SpriteSpecification {
		GUIElementReference parent;
		F32x2 textureScale;
		F32x2 textureOffset;
	};

	Sprite createSprite(GUIContext& guiContext, SpriteSpecification specification);
	void renderSprites(const std::span<Sprite*> sprites, CommandContext& context, GUIContext& guiContext, Window& window);
}
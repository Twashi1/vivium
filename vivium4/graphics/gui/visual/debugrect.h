#pragma once

#include "../base.h"
#include "../../color.h"
#include "context.h"

namespace Vivium {
	void debugRect(F32x2 position, F32x2 scale, Color color, GUIContext& context);

	void renderDebugRects(CommandContext& context, GUIContext& guiContext, Window& window);
}

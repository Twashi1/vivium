#include "debugrect.h"

namespace Vivium {
	void debugRect(F32x2 position, F32x2 scale, Color color, GUIContext& context)
	{
		_GUIDebugRectInstanceData rect;
		rect.position = position;
		rect.scale = scale;
		rect.borderColor = color;
		rect.borderSize = 0.01;

		context.debugRect.rects.push_back(rect);
	}
	
	void renderDebugRects(CommandContext& context, GUIContext& guiContext, Window& window)
	{
		Perspective perspective = orthogonalPerspective2D(windowDimensions(window), F32x2(0.0f), 0.0f, 1.0f);

		VIVIUM_ASSERT(guiContext.debugRect.rects.size() < guiContext.debugRect.MAX_DEBUG_RECTS, "Too many debug rects");

		setBuffer(guiContext.debugRect.storageBuffer.resource, 0, guiContext.debugRect.rects.data(), guiContext.debugRect.rects.size() * sizeof(_GUIDebugRectInstanceData));
		cmdBindPipeline(context, guiContext.debugRect.pipeline.resource);
		cmdBindVertexBuffer(context, guiContext.rectVertexBuffer.resource);
		cmdBindIndexBuffer(context, guiContext.rectIndexBuffer.resource);
		cmdBindDescriptorSet(context, guiContext.debugRect.descriptorSet.resource, guiContext.debugRect.pipeline.resource);
		cmdWritePushConstants(context, &perspective, sizeof(Perspective), 0, ShaderStage::VERTEX, guiContext.debugRect.pipeline.resource);
		cmdDrawIndexed(context, 6, guiContext.debugRect.rects.size());

		guiContext.debugRect.rects.clear();
	}
}
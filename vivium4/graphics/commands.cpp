#include "commands.h"

namespace Vivium {
	namespace Commands {
		namespace Context {
			Resource::Resource()
				: frameIndex(0), literalZero(0), inTransfer(false), transferPool(VK_NULL_HANDLE), transferCommandBuffer(VK_NULL_HANDLE)
			{}

			Resource::~Resource()
			{
				for (FunctionArray cleanupArray : perFrameCleanupArrays) {
					for (std::function<void(void)> function : cleanupArray) {
						function();
					}
				}
			}

			bool Resource::isNull() const
			{
				return transferPool == VK_NULL_HANDLE;
			}

			void Resource::addFunction(std::function<void(void)> function)
			{
				perFrameCleanupArrays[frameIndex].push_back(function);
			}

			void Resource::drop(Engine::Handle engine)
			{
				vkFreeCommandBuffers(engine->device, transferPool, 1, &transferCommandBuffer);
				vkDestroyCommandPool(engine->device, transferPool, nullptr);
			}
			
			void Resource::beginTransfer()
			{
				Commands::beginCommandBuffer(transferCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				inTransfer = true;
			}
			
			void Resource::endTransfer(Engine::Handle engine)
			{
				Commands::endCommandBuffer(&transferCommandBuffer, 1, engine->transferQueue);
				
				// TODO: bad
				vkQueueWaitIdle(engine->transferQueue);
				
				inTransfer = false;
			}
			
			void Resource::flush(Engine::Handle engine)
			{
				frameIndex = (frameIndex + 1) % 2;
				currentCommandBuffer = engine->commandBuffers[engine->currentFrameIndex];

				for (std::function<void(void)> function : perFrameCleanupArrays[frameIndex]) {
					function();
				}

				perFrameCleanupArrays[frameIndex].clear();
			}

			void flush(Handle context, Engine::Handle engine)
			{
				VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(context);

				context->flush(engine);
			}
		}
	}
}
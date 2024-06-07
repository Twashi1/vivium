#include "context.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				void setup(Handle handle, Commands::Context::Handle context, Engine::Handle engine)
				{
					float vertexData[] = {
						0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f,
						0.0f, 1.0f
					};

					uint16_t indexData[] = { 0, 1, 2, 2, 3, 0 };

					VkDeviceMemory temporaryMemory;
					VkBuffer stagingBuffer;
					void* stagingMapping;
					Commands::createOneTimeStagingBuffer(engine, &stagingBuffer, &temporaryMemory,
						std::max(8 * sizeof(float), 6 * sizeof(uint16_t)), &stagingMapping);

					Buffer resource;
					resource.buffer = stagingBuffer;
					resource.mapping = stagingMapping;

					Commands::Context::beginTransfer(context);

					std::memcpy(stagingMapping, vertexData, 8 * sizeof(float));
					Commands::transferBuffer(context, resource, 8 * sizeof(float), handle->button.vertexBuffer.resource);

					std::memcpy(stagingMapping, indexData, 6 * sizeof(uint16_t));
					Commands::transferBuffer(context, resource, 6 * sizeof(uint16_t), handle->button.indexBuffer.resource);

					Commands::Context::endTransfer(context, engine);

					Commands::freeOneTimeStagingBuffer(engine, stagingBuffer, temporaryMemory);

					drop(VIVIUM_NULL_STORAGE, handle->text.fragmentShader.resource, engine);
					drop(VIVIUM_NULL_STORAGE, handle->text.vertexShader.resource, engine);

					drop(VIVIUM_NULL_STORAGE, handle->button.fragmentShader.resource, engine);
					drop(VIVIUM_NULL_STORAGE, handle->button.vertexShader.resource, engine);

					// TODO: maybe the descriptor layout can be freed here?
				}
			}
		}
	}
}
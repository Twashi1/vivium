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

					Buffer::Resource resource;
					resource.buffer = stagingBuffer;
					resource.mapping = stagingMapping;
					resource.size = 8 * sizeof(float);

					Commands::Context::beginTransfer(context);
					std::memcpy(stagingMapping, vertexData, 8 * sizeof(float));
					Commands::transferBuffer(context, &resource, handle->button.vertexBuffer);

					std::memcpy(stagingMapping, indexData, 6 * sizeof(uint16_t));
					resource.size = 6 * sizeof(uint16_t);
					Commands::transferBuffer(context, &resource, handle->button.indexBuffer);

					Commands::Context::endTransfer(context, engine);

					Commands::freeOneTimeStagingBuffer(engine, stagingBuffer, temporaryMemory);

					Shader::drop(&handle->transientStorage, handle->text.fragmentShader, engine);
					Shader::drop(&handle->transientStorage, handle->text.vertexShader, engine);

					Shader::drop(&handle->transientStorage, handle->button.fragmentShader, engine);
					Shader::drop(&handle->transientStorage, handle->button.vertexShader, engine);

					// TODO: maybe the descriptor layout can be freed here?

					handle->transientStorage.free();
				}
			}
		}
	}
}
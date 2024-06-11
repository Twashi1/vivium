#include "context.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				void setup(Handle handle, ResourceManager::Static::Handle manager, Commands::Context::Handle context, Engine::Handle engine)
				{
					ResourceManager::Static::convertReference(manager, handle->text.pipeline);
					ResourceManager::Static::convertReference(manager, handle->text.descriptorLayout);
					ResourceManager::Static::convertReference(manager, handle->text.fragmentShader);
					ResourceManager::Static::convertReference(manager, handle->text.vertexShader);

					ResourceManager::Static::convertReference(manager, handle->button.pipeline);
					ResourceManager::Static::convertReference(manager, handle->button.descriptorLayout);
					ResourceManager::Static::convertReference(manager, handle->button.fragmentShader);
					ResourceManager::Static::convertReference(manager, handle->button.vertexShader);
					ResourceManager::Static::convertReference(manager, handle->button.storageBuffer);
					ResourceManager::Static::convertReference(manager, handle->button.descriptorSet);
					ResourceManager::Static::convertReference(manager, handle->button.vertexBuffer);
					ResourceManager::Static::convertReference(manager, handle->button.indexBuffer);

					float vertexData[] = {
						0.0f, 0.0f,
						1.0f, 0.0f,
						1.0f, 1.0f,
						0.0f, 1.0f
					};

					uint16_t indexData[] = { 0, 1, 2, 2, 3, 0 };

					// TODO: big error here, we schedule two moves from the same staging buffer, resulting in
					// an overwrite that gives bogus data!

					VkDeviceMemory temporaryMemory;
					VkBuffer stagingBuffer;
					void* stagingMapping;
					Commands::createOneTimeStagingBuffer(engine, &stagingBuffer, &temporaryMemory,
						8 * sizeof(float) + 6 * sizeof(uint16_t), &stagingMapping);

					Buffer resource;
					resource.buffer = stagingBuffer;
					resource.mapping = stagingMapping;

					Commands::Context::beginTransfer(context);

					std::memcpy(stagingMapping, vertexData, 8 * sizeof(float));
					Commands::transferBuffer(context, resource, 8 * sizeof(float), 0, handle->button.vertexBuffer.resource);

					std::memcpy(reinterpret_cast<uint8_t*>(stagingMapping) + 8 * sizeof(float), indexData, 6 * sizeof(uint16_t));
					Commands::transferBuffer(context, resource, 6 * sizeof(uint16_t), 8 * sizeof(float), handle->button.indexBuffer.resource);

					Commands::Context::endTransfer(context, engine);

					Commands::freeOneTimeStagingBuffer(engine, stagingBuffer, temporaryMemory);

					dropShader(VIVIUM_NULL_STORAGE, handle->text.fragmentShader.resource, engine);
					dropShader(VIVIUM_NULL_STORAGE, handle->text.vertexShader.resource, engine);

					dropShader(VIVIUM_NULL_STORAGE, handle->button.fragmentShader.resource, engine);
					dropShader(VIVIUM_NULL_STORAGE, handle->button.vertexShader.resource, engine);

					// TODO: maybe the descriptor layout can be freed here?
				}
			}
		}
	}
}
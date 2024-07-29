#pragma once

#include "../../../storage.h"
#include "../../resource_manager.h"
#include "../../color.h"
#include "../base.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				struct _ButtonInstanceData {
					F32x2 position; // 8 bytes
					F32x2 scale;    // 16 bytes
					Color foregroundColor; // 28 bytes
					float _fill0; // 32 bytes
				};

				// TODO: border size not in px
				struct _PanelInstanceData {
					F32x2 position;
					F32x2 scale;
					Color backgroundColor;
					float borderSizePx; // 32 bytes
					Color borderColor; // 44 bytes
					float _fill0;
				};

				struct Resource {
					Ref<Buffer> rectVertexBuffer;
					Ref<Buffer> rectIndexBuffer;

					struct {
						Ref<Pipeline> pipeline;
						Ref<DescriptorLayout> descriptorLayout;
						Ref<Shader> fragmentShader;
						Ref<Shader> vertexShader;
					} text;

					struct {
						static constexpr uint64_t MAX_BUTTONS = 128;

						Ref<Shader> fragmentShader;
						Ref<Shader> vertexShader;

						Ref<Buffer> storageBuffer;

						Ref<DescriptorLayout> descriptorLayout;
						Ref<DescriptorSet> descriptorSet;
						Ref<Pipeline> pipeline;
					} button;

					struct {
						static constexpr uint64_t MAX_PANELS = 128;

						Ref<Shader> fragmentShader;
						Ref<Shader> vertexShader;

						Ref<Buffer> storageBuffer;

						Ref<DescriptorLayout> descriptorLayout;
						Ref<DescriptorSet> descriptorSet;
						Ref<Pipeline> pipeline;
					} panel;

					GUIElement* defaultParent;
					Storage::Static::Pool elementStorage;
				};

				typedef Resource* Handle;
				typedef Resource* PromisedHandle;

				GUIElement* _allocateGUIElement(Context::Handle context);

				void _submitGenericContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window);
				void _submitTextContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window);
				void _submitButtonContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window);
				void _submitPanelContext(Handle handle, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window);

				template <Storage::StorageType StorageType>
				PromisedHandle submit(StorageType* allocator, ResourceManager::Static::Handle manager, Engine::Handle engine, Window::Handle window) {
					Handle handle = Storage::allocateResource<Resource>(allocator);

					// Generate the font if it doesn't exist
					if (!std::filesystem::exists("res/fonts/consola.sdf"))
					{
						Font::compileSignedDistanceField("res/fonts/consola.ttf", 512, "res/fonts/consola.sdf", 48, 1.0f);
					}

					handle->defaultParent = _allocateGUIElement(handle);

					_submitGenericContext(handle, manager, engine, window);
					_submitButtonContext(handle, manager, engine, window);
					_submitTextContext(handle, manager, engine, window);
					_submitPanelContext(handle, manager, engine, window);

					return handle;
				}
				
				void setup(Handle handle, ResourceManager::Static::Handle manager, Commands::Context::Handle context, Engine::Handle engine);

				void updateContext(Handle handle, F32x2 windowDimensions);

				template <Storage::StorageType StorageType>
				void drop(StorageType* allocator, Handle handle, Engine::Handle engine) {
					VIVIUM_CHECK_HANDLE_EXISTS(handle);

					dropDescriptorLayout(handle->text.descriptorLayout.resource, engine);
					dropPipeline(handle->text.pipeline.resource, engine);

					dropDescriptorLayout(handle->button.descriptorLayout.resource, engine);
					dropDescriptorLayout(handle->panel.descriptorLayout.resource, engine);
					dropPipeline(handle->button.pipeline.resource, engine);
					dropPipeline(handle->panel.pipeline.resource, engine);

					dropBuffer(handle->button.storageBuffer.resource, engine);
					dropBuffer(handle->panel.storageBuffer.resource, engine);

					dropBuffer(handle->rectVertexBuffer.resource, engine);
					dropBuffer(handle->rectIndexBuffer.resource, engine);

					Storage::dropResource(allocator, handle);
				}
			}
		}
	}
}
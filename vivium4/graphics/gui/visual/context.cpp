#include "context.h"

namespace Vivium {
	namespace GUI {
		namespace Visual {
			namespace Context {
				void clean(Handle handle, Engine::Handle engine)
				{
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
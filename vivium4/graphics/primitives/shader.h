#pragma once

#include "../../core.h"

namespace Vivium {
	namespace Graphics {
		namespace Shader {
			enum class DataType {
				BOOL,
				INT,
				UINT,
				FLOAT,
				DOUBLE,

				BVEC2,
				IVEC2,
				UVEC2,
				VEC2,
				DVEC2,

				BVEC3,
				IVEC3,
				UVEC3,
				VEC3,
				/* DVEC3 */

				BVEC4,
				IVEC4,
				UVEC4,
				VEC4,
				/* DVEC4 */
			};

			uint32_t sizeOf(DataType type);
			VkFormat formatOf(DataType type);
		}
	}
}
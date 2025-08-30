#pragma once

#include <filesystem>
#include <fstream>

#include "../../core.h"
#include "../../engine.h"
#include "../../storage.h"

namespace Vivium {
	enum class ShaderDataType : uint64_t {
		BOOL	= (1ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R8_UINT),
		INT		= (4ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32_SINT),
		UINT	= (4ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32_UINT),
		FLOAT	= (4ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32_SFLOAT),
		DOUBLE	= (8ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R64_SFLOAT),

		BVEC2	= (2ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R8G8_UINT),
		IVEC2	= (8ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32_SINT),
		UVEC2	= (8ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32_UINT),
		VEC2	= (8ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32_SFLOAT),
		DVEC2	= (16ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R64G64_SINT),

		BVEC3	= (3ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R8G8B8_UINT),
		IVEC3	= (12ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_SINT),
		UVEC3	= (12ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_UINT),
		VEC3	= (12ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_SFLOAT),
		/* DVEC3 */

		BVEC4	= (4ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R8G8B8A8_UINT),
		IVEC4	= (16ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_SINT),
		UVEC4	= (16ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_UINT),
		VEC4	= (16ULL << 32ULL) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_SFLOAT),
		/* DVEC4 */
	};

	uint32_t _sizeOfShaderDataType(ShaderDataType type);
	VkFormat _formatOfShaderDataType(ShaderDataType type);

	char const* getString(ShaderDataType type);

	enum class ShaderStage {
		VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
		FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
		GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT
	};

	ShaderStage operator|(ShaderStage lhs, ShaderStage rhs);
	char const* getString(ShaderStage stage);

	struct ShaderSpecification {
		ShaderStage stage;
		std::string code;
		uint32_t length;
	};

	struct Shader {
		VkShaderModule shader;
	};

	struct ShaderReference {
		uint64_t referenceIndex;
	};

	void dropShader(Shader& shader, Engine& engine);

	// TODO: returning the specification is convenient, but promotes bad practice
	ShaderSpecification compileShader(ShaderStage stage, const char* sourceFilename, const char* destFilename);
}

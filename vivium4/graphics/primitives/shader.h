#pragma once

#include <filesystem>
#include <fstream>

#include "../../core.h"
#include "../../engine.h"
#include "../../storage.h"

namespace Vivium {
	enum class ShaderDataType : uint64_t {
		BOOL	= (1Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R8_UINT),
		INT		= (4Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32_SINT),
		UINT	= (4Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32_UINT),
		FLOAT	= (4Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32_SFLOAT),
		DOUBLE	= (8Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R64_SFLOAT),

		BVEC2	= (2Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R8G8_UINT),
		IVEC2	= (8Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32_SINT),
		UVEC2	= (8Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32_UINT),
		VEC2	= (8Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32_SFLOAT),
		DVEC2	= (16Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R64G64_SINT),

		BVEC3	= (3Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R8G8B8_UINT),
		IVEC3	= (12Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_SINT),
		UVEC3	= (12Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_UINT),
		VEC3	= (12Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32_SFLOAT),
		/* DVEC3 */

		BVEC4	= (4Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R8G8B8A8_UINT),
		IVEC4	= (16Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_SINT),
		UVEC4	= (16Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_UINT),
		VEC4	= (16Ui64 << 32Ui64) | static_cast<uint64_t>(VK_FORMAT_R32G32B32A32_SFLOAT),
		/* DVEC4 */
	};

	uint32_t _sizeOfShaderDataType(ShaderDataType type);
	VkFormat _formatOfShaderDataType(ShaderDataType type);

	enum class ShaderStage {
		VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
		FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
		GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT
	};

	ShaderStage operator|(ShaderStage lhs, ShaderStage rhs);

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

	template <Storage::StorageType StorageType>
	void dropShader(StorageType* allocator, Shader& shader, Engine::Handle engine) {
		vkDestroyShaderModule(engine->device, shader.shader, nullptr);

		Storage::dropResource(allocator, &shader);
	}

	// TODO: returning the specification is convenient, but promotes bad practice
	ShaderSpecification compileShader(ShaderStage stage, const char* sourceFilename, const char* destFilename);
}
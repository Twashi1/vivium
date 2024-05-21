#pragma once

#include <filesystem>
#include <fstream>

#include "../../core.h"
#include "../../engine.h"
#include "../../storage.h"

namespace Vivium {
	namespace Shader {
		enum class DataType : uint64_t {
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

		uint32_t sizeOf(DataType type);
		VkFormat formatOf(DataType type);

		enum class Stage {
			VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
			FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
			GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT
		};

		Stage operator|(Stage lhs, Stage rhs);

		struct Specification {
			Stage stage;
			std::string code;
			uint32_t length;

			Specification() = default;
			Specification(Stage stage, std::string code, uint32_t length);
		};

		struct Resource {
			VkShaderStageFlagBits flags;
			VkShaderModule shader;
		};

		typedef Resource* Handle;

		template <Allocator::AllocatorType AllocatorType>
		Handle create(AllocatorType* allocator, Engine::Handle engine, Specification specification) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);

			Handle handle = Allocator::allocateResource<Resource>(allocator);

			{
				handle->flags = static_cast<VkShaderStageFlagBits>(specification.stage);

				VkShaderModuleCreateInfo shaderCreateInfo{};
				shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderCreateInfo.codeSize = specification.length;
				shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(specification.code.c_str());

				VIVIUM_VK_CHECK(vkCreateShaderModule(
					engine->device,
					&shaderCreateInfo,
					nullptr,
					&handle->shader
				), "Failed to create shader module"
				);
			}

			return handle;
		}

		template <Allocator::AllocatorType AllocatorType>
		void drop(AllocatorType* allocator, Shader::Handle shader, Engine::Handle engine) {
			VIVIUM_CHECK_RESOURCE_EXISTS_AT_HANDLE(engine, Engine::isNull);
			VIVIUM_CHECK_HANDLE_EXISTS(shader);

			vkDestroyShaderModule(engine->device, shader->shader, nullptr);

			Allocator::dropResource(allocator, shader);
		}

		// TODO: returning the specification is convenient, but promotes bad practice
		Specification compile(Shader::Stage stage, const char* sourceFilename, const char* destFilename);
	}
}
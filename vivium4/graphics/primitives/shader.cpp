#include "shader.h"

namespace Vivium {
	namespace Shader {
		Stage operator|(Stage lhs, Stage rhs)
		{
			return static_cast<Stage>(static_cast<int>(lhs) | static_cast<int>(rhs));
		}

		// TODO: really shouldn't do this
		Specification compile(Shader::Stage stage, const char* sourceFilename, const char* destFilename) {
			if (!std::filesystem::exists(VIVIUM_GLSLC_PATH)) {
				VIVIUM_LOG(Log::FATAL, "Couldn't find glslc.exe for compiling shaders");
			}

			std::string stageOption;

			switch (stage) {
			case Shader::Stage::FRAGMENT:
				stageOption = "-fs";
			}
			 
			// TODO: prefer CreateProcess https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
			std::string fullCommand = std::format("if 1==1 \"{}\" {} -o {}", VIVIUM_GLSLC_PATH, sourceFilename, destFilename);
			system(fullCommand.c_str());

			std::ifstream shaderBinaryFile;
			shaderBinaryFile.open(destFilename, std::ios::binary);

			if (!shaderBinaryFile.is_open())
				VIVIUM_LOG(Log::FATAL, "Failed to open shader binary file");

			std::string binaryCode;
			shaderBinaryFile.seekg(0, std::ios::end);
			binaryCode.resize(shaderBinaryFile.tellg());
			shaderBinaryFile.seekg(0, std::ios::beg);
			shaderBinaryFile.read(binaryCode.data(), binaryCode.size());

			shaderBinaryFile.close();

			return Specification(stage, binaryCode, binaryCode.size());
		}

		uint32_t sizeOf(DataType type)
		{
			return static_cast<uint32_t>(static_cast<uint64_t>(type) >> 32Ui64);
		}
			
		VkFormat formatOf(DataType type)
		{
			return static_cast<VkFormat>(static_cast<uint32_t>(type));
		}

		bool Resource::isNull() const
		{
			return shader == VK_NULL_HANDLE;
		}

		void Resource::drop(Engine::Handle engine)
		{
			vkDestroyShaderModule(engine->device, shader, nullptr);
		}

		void Resource::create(Engine::Handle engine, Specification specification)
		{
			flags = static_cast<VkShaderStageFlagBits>(specification.stage);

			VkShaderModuleCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderCreateInfo.codeSize = specification.length;
			shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(specification.code.c_str());

			VIVIUM_VK_CHECK(vkCreateShaderModule(
				engine->device,
				&shaderCreateInfo,
				nullptr,
				&shader
			), "Failed to create shader module"
			);
		}
		
		Specification::Specification(Stage stage, std::string code, uint32_t length)
			: stage(stage), code(code), length(length)
		{}
	}
}
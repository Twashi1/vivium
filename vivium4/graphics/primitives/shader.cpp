#include "shader.h"

namespace Vivium {
	ShaderStage operator|(ShaderStage lhs, ShaderStage rhs)
	{
		return static_cast<ShaderStage>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	ShaderSpecification compileShader(ShaderStage stage, const char* sourceFilename, const char* destFilename) {
		if (!std::filesystem::exists(VIVIUM_GLSLC_PATH)) {
			VIVIUM_LOG(Log::FATAL, "Couldn't find glslc.exe for compiling shaders");
		}

		std::string stageOption;

		switch (stage) {
		case ShaderStage::FRAGMENT:
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

		return ShaderSpecification(stage, binaryCode, binaryCode.size());
	}

	uint32_t _sizeOfShaderDataType(ShaderDataType type)
	{
		// High 32 bits
		return static_cast<uint32_t>(static_cast<uint64_t>(type) >> 32Ui64);
	}
			
	VkFormat _formatOfShaderDataType(ShaderDataType type)
	{
		// Low 32 bits
		return static_cast<VkFormat>(static_cast<uint64_t>(type) & ((1Ui64 << 32Ui64) - 1));
	}
}
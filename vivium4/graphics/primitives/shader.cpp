#include "shader.h"

namespace Vivium {
ShaderStage operator|(ShaderStage lhs, ShaderStage rhs) {
  return static_cast<ShaderStage>(static_cast<int>(lhs) |
                                  static_cast<int>(rhs));
}

ShaderSpecification compileShader(ShaderStage stage, const char* sourceFilename,
                                  const char* destFilename) {
  if (!std::filesystem::exists(VIVIUM_GLSLC_PATH)) {
    VIVIUM_LOG(
        LogSeverity::FATAL,
        "Couldn't find glslc binary for compiling shaders, expected at {}",
        VIVIUM_GLSLC_PATH);
  }

  std::string stageOption;

  switch (stage) {
    case ShaderStage::FRAGMENT:
      stageOption = "-fs";
      break;
    default:
      break;
  }

  // TODO: prefer CreateProcess
  // https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
  // TODO: test works cross platform, prefer the dedicated unix/windows
  // functions for starting processes
  std::string fullCommand = std::format(
      "\"{}\" {} -o {} 2>&1", VIVIUM_GLSLC_PATH, sourceFilename, destFilename);
  system(fullCommand.c_str());

  std::ifstream shaderBinaryFile;
  shaderBinaryFile.open(destFilename, std::ios::binary);

  if (!shaderBinaryFile.is_open())
    VIVIUM_LOG(LogSeverity::FATAL,
               "Failed to open shader binary file, does file exist at {}? Does "
               "original exist at {}",
               destFilename, sourceFilename);

  std::string binaryCode;
  // Get length before reading
  shaderBinaryFile.seekg(0, std::ios::end);
  binaryCode.resize(shaderBinaryFile.tellg());
  shaderBinaryFile.seekg(0, std::ios::beg);
  shaderBinaryFile.read(binaryCode.data(), binaryCode.size());

  shaderBinaryFile.close();

  return ShaderSpecification(stage, binaryCode, binaryCode.size());
}

uint32_t _sizeOfShaderDataType(ShaderDataType type) {
  // High 32 bits
  return static_cast<uint32_t>(static_cast<uint64_t>(type) >> 32ULL);
}

VkFormat _formatOfShaderDataType(ShaderDataType type) {
  // Low 32 bits
  return static_cast<VkFormat>(static_cast<uint64_t>(type) &
                               ((1ULL << 32ULL) - 1));
}

char const* getString(ShaderDataType type) {
  switch (type) {
    case ShaderDataType::BOOL:
      return "Bool";
    case ShaderDataType::INT:
      return "Int";
    case ShaderDataType::UINT:
      return "UInt";
    case ShaderDataType::FLOAT:
      return "Float";
    case ShaderDataType::DOUBLE:
      return "Double";
    case ShaderDataType::BVEC2:
      return "BVec2";
    case ShaderDataType::IVEC2:
      return "IVec2";
    case ShaderDataType::UVEC2:
      return "UVec2";
    case ShaderDataType::VEC2:
      return "Vec2";
    case ShaderDataType::DVEC2:
      return "DVec2";
    case ShaderDataType::BVEC3:
      return "BVec3";
    case ShaderDataType::IVEC3:
      return "IVec3";
    case ShaderDataType::UVEC3:
      return "UVec3";
    case ShaderDataType::VEC3:
      return "Vec3";
    case ShaderDataType::BVEC4:
      return "BVec4";
    case ShaderDataType::IVEC4:
      return "IVec4";
    case ShaderDataType::UVEC4:
      return "UVec4";
    case ShaderDataType::VEC4:
      return "Vec4";
    default:
      return "Unknown";
  }
}

char const* getString(ShaderStage stage) {
  switch (stage) {
    case ShaderStage::VERTEX:
      return "Vertex";
    case ShaderStage::FRAGMENT:
      return "Fragment";
    case ShaderStage::GEOMETRY:
      return "Geometry";
    default:
      return "Unknown";
  }
}

void dropShader(Shader& shader, Engine& engine) {
  vkDestroyShaderModule(engine.device, shader.shader, nullptr);
}
}  // namespace Vivium

#pragma once

#include "../graphics/resource_manager.h"

namespace Vivium {
struct ShadebugStorageBuffer {
  uint64_t size;
  std::string name;
  ShaderDataType type;
};

struct ShadebugSpecification {
  std::vector<ShadebugStorageBuffer> storageBuffers;
  // Modified shader
  ShaderSpecification shaderSpec;
};

// TODO: the required buffers/descriptors to be added into the pipeline creation
struct ShadebugContext {
  std::vector<Ref<Buffer>> storageBuffers;
  std::vector<Ref<DescriptorSet>> descriptorSets;
  std::unordered_map<std::string, uint64_t> resourceMap;

  Ref<Shader> instrumentedShader;
};

struct ShadebugOutput {
  uint64_t size;
  void const* data;
  ShaderDataType type;
};

ShadebugSpecification shadebugInstrument(std::string shaderCode,
                                         ShaderStage stage);
ShadebugSpecification shadebugInstrumentFile(std::string filename,
                                             ShaderStage stage);
ShadebugContext shadebugAllocate(ShadebugSpecification const& spec,
                                 ResourceManager& manager);
ShadebugOutput shadebugRead(ShadebugContext const& context, std::string name);

}  // namespace Vivium

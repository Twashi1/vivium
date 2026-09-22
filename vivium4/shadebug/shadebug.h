#pragma once

#include "../graphics/resource_manager.h"
#include "../storage.h"

namespace Vivium {
struct ShadebugLexerContext {
  std::string code;
  uint64_t position;
  uint64_t tokenBegin;
  uint64_t lineNumber;

  BlockAllocator allocator;
};

enum class ShadebugTokenType : uint16_t {
  INDIRECT_MASK = (1 << 15),
  NONE = 0,
  IDENTIFIER = INDIRECT_MASK | (NONE + 1),
  LOCATION,
  BINDING,
  OUT,
  IN,
  INT_LITERAL,
  FLOAT_LITERAL,
  FRAG_COORD,
  STATEMENT_END,
  TYPE_IDENTIFIER = INDIRECT_MASK | (NONE + 1),
  LPAREN,
  RPAREN,
  LCURLY,
  RCURLY,
  LSQUARE,
  RSQUARE,
  COMMA,
  OPERATOR,
  MACRO,
  EQUALS,
};

struct ShadebugToken {
  union ShadebugTokenData {
    void* ptr;
    std::array<uint8_t, 8> inplace;
  } data;

  ShadebugTokenType type;

  uint32_t position;
  uint32_t lineNumber;
  uint32_t tokenLength;
};

// TODO: move to its own class
struct TriviallyRelocateableString {
  char* address;
  uint64_t size;
  std::array<uint8_t, 16> sso;
};

void setTriviallyRelocateableString(TriviallyRelocateableString& string,
                                    std::string_view const value);
std::string_view readTriviallyRelocateableString(
    TriviallyRelocateableString const& string);

void _populateTokenDebug(ShadebugLexerContext const& context,
                         ShadebugToken& token);

template <ShadebugTokenType _TokenType, typename T>
ShadebugToken _createShadebugToken(ShadebugLexerContext& context,
                                   T const& value);
template <ShadebugTokenType _TokenType>
ShadebugToken _createShadebugToken(ShadebugLexerContext& context,
                                   std::string_view const value);

ShadebugLexerContext _createShadebugLexerContext(std::string shaderCode);
void _dropShadebugLexerContext(ShadebugLexerContext& context);
ShadebugToken _getNextToken(ShadebugLexerContext& context);
char _nextChar(ShadebugLexerContext& context);
char _viewFirst(ShadebugLexerContext const& context);
char _viewFollow(ShadebugLexerContext const& context);
uint64_t _collectDigits(ShadebugLexerContext& context);
ShadebugToken _parseNumber(ShadebugLexerContext& context);

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

// TODO: the required buffers/descriptors to be added into the pipeline
// creation
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

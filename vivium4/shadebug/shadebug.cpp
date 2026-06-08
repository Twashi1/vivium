#include "shadebug.h"

namespace Vivium {

ShadebugSpecification shadebugInstrument(std::string shaderCode,
                                         ShaderStage stage) {
  // Apply our preprocessing on the code to note the variables and their type
  // that we need to record
  // We do need to build a lexer/parser? or we make user specify additional data
  // for proof-of-concept, we will just use preprocessor for now
  for (uint64_t i = 0; i < shaderCode.size(); i++) {
    std::string line = "";
    char current = shaderCode[i];

    if (current != '\n') {
      line += current;
    }

    if (current == '\n' || i == shaderCode.size() - 1) {
      // run some code

      // TODO: more efficient lexing; can just use our own state system/DFA

      // not one of our preprocessors
      if (!line.starts_with("//!")) {
        continue;
      }

      if (line.starts_with("//! export")) {
        // do some more processing?
      }

      if (line.starts_with("//! here")) {
        // a
      }
    }
  }

  // TODO: also need to re-create some buffer region
  // TODO: would be much nicer to just do a full lexer/parser

  ShadebugSpecification spec;

  return spec;
}

ShadebugSpecification shadebugInstrumentFile(std::string filename,
                                             ShaderStage stage);
ShadebugContext shadebugAllocate(ShadebugSpecification const& spec,
                                 ResourceManager& manager);
ShadebugOutput shadebugRead(ShadebugContext const& context, std::string name);
}  // namespace Vivium

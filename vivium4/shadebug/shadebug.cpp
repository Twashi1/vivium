#include "shadebug.h"

#include "shadebug_impl.h"

namespace Vivium {

void _populateTokenDebug(ShadebugLexerContext const& context,
                         ShadebugToken& token) {
  token.lineNumber = context.lineNumber;
  token.tokenLength = context.position - context.tokenBegin;
  token.position = context.tokenBegin;
}

ShadebugLexerContext _createShadebugLexerContext(std::string shaderCode) {
  ShadebugLexerContext context;
  context.code = shaderCode;
  context.lineNumber = 0;
  context.tokenBegin = 0;
  context.position = 0;
  context.allocator = createBlockAllocator(4096);

  return context;
}

void setTriviallyRelocateableString(TriviallyRelocateableString& string,
                                    std::string_view const value) {}

std::string_view readTriviallyRelocateableString(
    TriviallyRelocateableString const& string);

void _dropShadebugLexerContext(ShadebugLexerContext& context) {
  dropBlockAllocator(context.allocator);
}

char _nextChar(ShadebugLexerContext& context) {
  if (context.position < context.code.size()) {
    return context.code[context.position++];
  }

  return '\0';
}

char _viewFirst(ShadebugLexerContext const& context) {
  if (context.position < context.code.size()) {
    return context.code[context.position];
  }

  return '\0';
}

char _viewFollow(ShadebugLexerContext const& context) {
  if (context.position + 1 < context.code.size()) {
    return context.code[context.position + 1];
  }

  return '\0';
}

uint64_t _collectDigits(ShadebugLexerContext& context) {
  uint64_t digits = 0;
  char currentChar = _viewFirst(context);

  while (std::isdigit(currentChar)) {
    digits *= 10;
    digits += static_cast<uint8_t>(currentChar) - static_cast<uint8_t>('0');

    currentChar = _nextChar(context);
  }

  return digits;
}

ShadebugToken _parseNumber(ShadebugLexerContext& context) {
  char currentChar = _viewFirst(context);

  // TODO: do a regex match on the expression
  // determine whether integer/float
  // if float, then use relevant function to convert to float
  // note that we don't actually need to parse the correct value for float
  // literals
  uint64_t mantissa = _collectDigits(context);

  bool isDecimal = false;

  // Decimal number
  if (currentChar == '.') {
    isDecimal = true;

    currentChar = _nextChar(context);
  }

  // TODO: wrong
  uint64_t placeValue = 0;
  uint64_t decimals = 0;

  while (std::isdigit(currentChar)) {
    decimals *= 10;
    decimals += static_cast<uint8_t>(currentChar) - static_cast<uint8_t>('0');
    ++placeValue;

    currentChar = _nextChar(context);
  }

  bool isScientific = false;

  // Scientific notation (TODO: check compiler optimises this)
  if (currentChar == 'e' || currentChar == 'E') {
    isScientific = true;

    currentChar = _nextChar(context);
  }

  uint64_t exponent = _collectDigits(context);

  if (isDecimal) {
    // Floating-point path
    // TODO
  }

  // Integer math, just need to consider isScientific
  uint64_t finalValue = mantissa;

  if (isScientific) {
    finalValue *= integerPower(10, exponent);
  }

  ShadebugToken token =
      _createShadebugToken<ShadebugTokenType::INT_LITERAL, uint64_t>(
          context, finalValue);

  return token;
};

ShadebugToken _getNextToken(ShadebugLexerContext& context) {
  char currentChar = _nextChar(context);

  // Skip whitespace
  while (currentChar == ' ' || currentChar == '\t' || currentChar == '\n') {
    if (currentChar == '\n') {
      ++context.lineNumber;
    }

    currentChar = _nextChar(context);
  }

  if (currentChar == '.' && std::isdigit(_viewFollow(context))) {
    return _parseNumber(context);
  }

  if (std::isdigit(currentChar)) {
    return _parseNumber(context);
  }

  switch (currentChar) {}

  ShadebugToken currentToken;

  currentToken.position = context.position;
  currentToken.lineNumber = context.lineNumber;
  currentToken.type = ShadebugTokenType::NONE;

  return currentToken;
}

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

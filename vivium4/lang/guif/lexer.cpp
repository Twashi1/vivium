#include "lexer.h"

namespace Vivium {
namespace GUIF {

LexerContext createLexer(std::string_view text) {
  LexerContext context;
  context.code = text;
  context.pos = 0;
  context.line = 0;
  context.allocator = createBlockAllocator(TOKEN_BLOCK_SIZE);

  return context;
}

void dropLexer(LexerContext& context) { dropBlockAllocator(context.allocator); }

std::string tokenTypeString(TokenType type) {
  switch (type) {
    case TokenType::SQUARE_RIGHT:
      return "SQUARE_RIGHT";
    case TokenType::SQUARE_LEFT:
      return "SQUARE_LEFT";
    case TokenType::PAREN_LEFT:
      return "PAREN_LEFT";
    case TokenType::PAREN_RIGHT:
      return "PAREN_RIGHT";
    case TokenType::CURLY_LEFT:
      return "CURLY_LEFT";
    case TokenType::CURLY_RIGHT:
      return "CURLY_RIGHT";
    case TokenType::EQUAL:
      return "EQUAL";
    case TokenType::COLON:
      return "COLON";
    case TokenType::INTEGER:
      return "INTEGER";
    case TokenType::FLOATING:
      return "FLOATING";
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::PLUS:
      return "PLUS";
    case TokenType::SUBTRACT:
      return "SUBTRACT";
    case TokenType::MULTIPLY:
      return "MULTIPLY";
    case TokenType::DIVIDE:
      return "DIVIDE";
    default:
      return "UNKNOWN";
  }
}

Token readNumber(LexerContext& context) {
  bool isDecimal = false;
  uint64_t startIndex = context.pos;
  // Exclusive
  uint64_t endIndex = context.pos + 1;

  while (std::isdigit(context.currentChar) ||
         (context.currentChar == '.' && !isDecimal)) {
    endIndex += 1;
    advanceCharacter(context);
  }

  std::string_view number = context.code.substr(startIndex, endIndex);

  if (!isDecimal) {
    int64_t value = std::stoll(std::string(number));
    return createToken(TokenType::INTEGER, &value);
  }

  double value = std::stod(std::string(number));
  return createToken(TokenType::FLOATING, &value);
}

Token readIdentifier(LexerContext& context) {
  uint64_t startIndex = context.pos;
  uint64_t endIndex = context.pos + 1;

  while (std::isalnum(context.currentChar) || context.currentChar == '_') {
    endIndex += 1;
    advanceCharacter(context);
  }

  std::string identifier =
      std::string(context.code.substr(startIndex, endIndex));
  return createToken(TokenType::IDENTIFIER, &identifier);
}

Token advanceToken(LexerContext& context) {
  // TODO: implement
  while (context.currentChar) {
    // Identifier
    if (std::isalpha(context.currentChar)) {
      return readIdentifier(context);
    }

    if (std::isdigit(context.currentChar) || context.currentChar == '.') {
      return readNumber(context);
    }

    if (context.currentChar == '+') {
      advanceCharacter(context);
      return createToken(TokenType::PLUS, nullptr);
    }

    if (context.currentChar == '-') {
      advanceCharacter(context);
      return createToken(TokenType::SUBTRACT, nullptr);
    }

    if (context.currentChar == '*') {
      advanceCharacter(context);
      return createToken(TokenType::MULTIPLY, nullptr);
    }

    if (context.currentChar == '/') {
      advanceCharacter(context);
      return createToken(TokenType::DIVIDE, nullptr);
    }

    context.currentChar = advanceCharacter(context);
  }

  return createToken(TokenType::UNKNOWN, nullptr);
}

char advanceCharacter(LexerContext& context) {
  if (context.pos + 1 >= context.code.length()) {
    return '\0';
  }

  if (context.currentChar == '\n') {
    context.line += 1;
  }

  context.pos += 1;
  context.currentChar = context.code[context.pos];

  return '\0';
}

char peekCharacter(LexerContext const& context) {
  if (context.pos + 1 < context.code.length()) {
    return context.code[context.pos + 1];
  }

  return '\0';
}

Token expectToken(LexerContext& context, TokenType type) {
  Token token = advanceToken(context);

  VIVIUM_ASSERT(token.type == type,
                "Invalid token, expected type {} but got {}",
                tokenTypeString(type), tokenTypeString(token.type));

  return token;
}

Token createToken(TokenType type, void* data) {
  Token token;
  token.type = type;

  int64_t* a = nullptr;
  uint64_t* my_ptr = reinterpret_cast<uint64_t*>(a);

  switch (type) {
    case TokenType::SQUARE_RIGHT:
    case TokenType::SQUARE_LEFT:
    case TokenType::PAREN_LEFT:
    case TokenType::PAREN_RIGHT:
    case TokenType::CURLY_LEFT:
    case TokenType::CURLY_RIGHT:
    case TokenType::EQUAL:
    case TokenType::COLON:
    case TokenType::PLUS:
    case TokenType::SUBTRACT:
    case TokenType::MULTIPLY:
    case TokenType::DIVIDE:
      token.address = nullptr;
      break;
    case TokenType::INTEGER:
      *token.inplace = *reinterpret_cast<uint64_t*>(data);
      break;
    case TokenType::FLOATING:
      *token.inplace = *reinterpret_cast<double*>(data);
      break;
    case TokenType::IDENTIFIER:
      *token.inplace = *reinterpret_cast<std::string*>(data);
      break;
    default:
      token.address = nullptr;
      break;
  }

  return token;
}
}  // namespace GUIF
}  // namespace Vivium

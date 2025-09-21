#include "lexer.h"

namespace Vivium {
namespace GUIF {

LexerContext createLexer(std::string_view text) {
  LexerContext context;
  context.code = text;
  context.pos = 0;
  context.line = 0;
  context.currentChar = '\0';
  context.allocator = createBlockAllocator(TOKEN_BLOCK_SIZE);
  context.currentToken =
      createToken(context.allocator, TokenType::UNKNOWN, nullptr);

  if (context.code.length() > 0) {
    context.currentChar = context.code[0];
  }

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
    return createToken(context.allocator, TokenType::INTEGER, &value);
  }

  double value = std::stod(std::string(number));
  return createToken(context.allocator, TokenType::FLOATING, &value);
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

  return createToken(context.allocator, TokenType::IDENTIFIER, &identifier);
}

Token advanceToken(LexerContext& context) {
  context.currentToken = _getNextToken(context);
  return context.currentToken;
}

Token _getNextToken(LexerContext& context) {
  // TODO: implement
  while (context.currentChar) {
    if (std::isspace(context.currentChar) || context.currentChar == '\n') {
      advanceCharacter(context);
      continue;
    }

    // Line comment
    if (context.currentChar == '#') {
      while (context.currentChar != '\n') {
        advanceCharacter(context);
      }
    }

    // Identifier
    if (std::isalpha(context.currentChar) || context.currentChar == '_') {
      return readIdentifier(context);
    }

    if (std::isdigit(context.currentChar) || context.currentChar == '.') {
      return readNumber(context);
    }

    switch (context.currentChar) {
      case '+':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::PLUS, nullptr);
      case '-':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::SUBTRACT, nullptr);
      case '*':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::MULTIPLY, nullptr);
      case '/':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::DIVIDE, nullptr);
      case ',':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::COMMA, nullptr);
      case ':':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::COLON, nullptr);
      case '(':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::PAREN_LEFT, nullptr);
      case ')':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::PAREN_RIGHT, nullptr);
      case '{':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::CURLY_LEFT, nullptr);
      case '}':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::CURLY_RIGHT, nullptr);
      case '[':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::SQUARE_LEFT, nullptr);
      case ']':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::SQUARE_RIGHT, nullptr);
      case ';':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::SEMICOLON, nullptr);
      default:
        break;
    }

    VIVIUM_LOG(LogSeverity::ERROR,
               "Unknown token on line {} at pos {}, character {}", context.line,
               context.pos, context.currentChar);

    return createToken(context.allocator, TokenType::UNKNOWN, nullptr);
  }

  return createToken(context.allocator, TokenType::END_OF_FILE, nullptr);
}

char advanceCharacter(LexerContext& context) {
  if (context.pos + 1 >= context.code.length()) {
    context.currentChar = '\0';
    return context.currentChar;
  }

  if (context.currentChar == '\n') {
    context.line += 1;
  }

  context.pos += 1;
  context.currentChar = context.code[context.pos];

  return context.currentChar;
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

Token createToken(BlockAllocator& allocator, TokenType type, void const* data) {
  Token token;
  token.type = type;

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
    case TokenType::END_OF_FILE:
      token.address = nullptr;
      break;
    case TokenType::INTEGER:
      new (token.inplace) int64_t(*reinterpret_cast<int64_t const*>(data));
      break;
    case TokenType::FLOATING:
      new (token.inplace) double(*reinterpret_cast<double const*>(data));
      break;
    case TokenType::IDENTIFIER:
      // Need to make a copy of the std::string and construct
      //  a std::string object in place
      new (token.inplace)
          std::string(*reinterpret_cast<std::string const*>(data));
      break;
    case TokenType::UNKNOWN:
      token.address = nullptr;
      break;
    default:
      token.address = nullptr;
      VIVIUM_LOG(LogSeverity::ERROR,
                 "Unknown token type passed to create token, value: {}",
                 static_cast<uint64_t>(type));
      break;
  }

  return token;
}
}  // namespace GUIF
}  // namespace Vivium

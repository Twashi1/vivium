#include "lexer.h"

namespace Vivium {
namespace GUIF {
Token copyToken(Token const token) {
  switch (token.type) {
    case TokenType::IDENTIFIER: {
      Token copyToken;
      copyToken.type = token.type;
      TokenIdentifier* identifier =
          new (copyToken.memory.data()) TokenIdentifier();
      *identifier = copyTokenString(
          *reinterpret_cast<TokenIdentifier const*>(token.memory.data()));

      VIVIUM_LOG(LogSeverity::DEBUG, "Copied token to have [{}], original [{}]",
                 getTokenString(*reinterpret_cast<TokenIdentifier const*>(
                     copyToken.memory.data())),
                 getTokenString(*reinterpret_cast<TokenIdentifier const*>(
                     token.memory.data())));

      return copyToken;
    }
    case TokenType::INTEGER: {
      Token copyToken;
      copyToken.type = token.type;
      new (copyToken.memory.data())
          int64_t(*reinterpret_cast<int64_t const*>(token.memory.data()));

      return copyToken;
    }
    case TokenType::FLOATING: {
      Token copyToken;
      copyToken.type = token.type;
      new (copyToken.memory.data()) double(
          *reinterpret_cast<double const*>(token.memory.data()));

      return copyToken;
    }
    default:
      return token;
  }
}

TokenString createTokenString(std::string_view string) {
  TokenString tokenString;
  tokenString.size = string.size();

  if (string.size() < tokenString.sso.size()) {
    tokenString.address = reinterpret_cast<char*>(tokenString.sso.data());
  } else {
    tokenString.address = new char[string.size() + 1];
  }

  std::memcpy(tokenString.address, string.data(),
              string.size() * sizeof(uint8_t));
  // Write null termination character
  tokenString.address[string.size()] = '\0';

  return tokenString;
}

void dropTokenString(TokenString const& string) {
  VIVIUM_LOG(LogSeverity::DEBUG, "Destroying token string");

  if (string.size >= string.sso.size()) {
    VIVIUM_LOG(LogSeverity::DEBUG,
               "Destroying token string that was heap allocated");
    delete[] string.address;
  }
}

TokenString copyTokenString(TokenString const& string) {
  // TODO: mostly hoping compiler fixes all the extra,
  //  but we should implement this more efficiently ourselves
  return createTokenString(getTokenString(string));
}

std::string getTokenString(TokenString const& string) {
  // Tricky line of code
  //  if we do a member-by-member copy, the address field points
  //  to the SSO of the other string
  //  So we need to update it
  //    essentially we can't trust the address field
  char const* address = string.address;

  if (string.size < string.sso.size()) {
    address = reinterpret_cast<char const*>(string.sso.data());
  }

  return std::string(address, string.size);
}

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

  context.keywords.insert({"if", TokenType::IF});
  context.keywords.insert({"else", TokenType::ELSE});
  context.keywords.insert({"as", TokenType::AS});
  context.keywords.insert({"while", TokenType::WHILE});
  context.keywords.insert({"for", TokenType::FOR});
  context.keywords.insert({"class", TokenType::CLASS});
  context.keywords.insert({"fn", TokenType::FUNCTION});
  context.keywords.insert({"override", TokenType::FUNCTION});

  return context;
}

void dropLexer(LexerContext& context) {
  destroyToken(context.currentToken);
  dropBlockAllocator(context.allocator);
}

std::string tokenTypeString(TokenType type) {
  switch (type) {
    case TokenType::UNKNOWN:
      return "UNKNOWN";
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::INTEGER:
      return "INTEGER";
    case TokenType::FLOATING:
      return "FLOATING";
    case TokenType::COLON:
      return "COLON";
    case TokenType::COMMA:
      return "COMMA";
    case TokenType::SEMICOLON:
      return "SEMICOLON";
    case TokenType::EQUAL:
      return "EQUAL";
    case TokenType::CURLY_LEFT:
      return "CURLY_LEFT";
    case TokenType::CURLY_RIGHT:
      return "CURLY_RIGHT";
    case TokenType::PAREN_LEFT:
      return "PAREN_LEFT";
    case TokenType::PAREN_RIGHT:
      return "PAREN_RIGHT";
    case TokenType::SQUARE_LEFT:
      return "SQUARE_LEFT";
    case TokenType::SQUARE_RIGHT:
      return "SQUARE_RIGHT";
    case TokenType::DOLLAR:
      return "DOLLAR";
    case TokenType::PLUS:
      return "PLUS";
    case TokenType::SUBTRACT:
      return "SUBTRACT";
    case TokenType::MULTIPLY:
      return "MULTIPLY";
    case TokenType::DIVIDE:
      return "DIVIDE";
    case TokenType::IF:
      return "IF";
    case TokenType::ELSE:
      return "ELSE";
    case TokenType::AS:
      return "AS";
    case TokenType::WHILE:
      return "WHILE";
    case TokenType::FOR:
      return "FOR";
    case TokenType::CLASS:
      return "CLASS";
    case TokenType::FUNCTION:
      return "FUNCTION";
    case TokenType::OVERRIDE:
      return "OVERRIDE";
    case TokenType::END_OF_FILE:
      return "END_OF_FILE";
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
  uint64_t endIndex = context.pos;

  while (std::isalnum(context.currentChar) || context.currentChar == '_') {
    ++endIndex;
    advanceCharacter(context);
  }

  std::string identifier =
      std::string(context.code.substr(startIndex, endIndex - startIndex));

  VIVIUM_LOG(LogSeverity::DEBUG, "Got identifier: [{}], indexing {} to {}",
             identifier, startIndex, endIndex);

  if (context.keywords.contains(identifier)) {
    return createToken(context.allocator, context.keywords.at(identifier),
                       nullptr);
  }

  return createToken(context.allocator, TokenType::IDENTIFIER, &identifier);
}

void advanceToken(LexerContext& context) {
  // Note we invalidate the memory of the last token with this
  destroyToken(context.currentToken);
  // TODO: do we need to copy token?
  context.currentToken = copyToken(_getNextToken(context));

  VIVIUM_LOG(LogSeverity::DEBUG, "Next token is of type: {}",
             tokenTypeString(context.currentToken.type));

  VIVIUM_LOG(LogSeverity::DEBUG, "Memory at current token:\n{}",
             getHexDump(&context.currentToken, sizeof(Token)));

  if (context.currentToken.type == TokenType::IDENTIFIER) {
    VIVIUM_LOG(LogSeverity::DEBUG, "Next token is an identifier of value [{}]",
               getTokenString(*reinterpret_cast<TokenIdentifier*>(
                   context.currentToken.memory.data())));
  }
}

Token peekToken(LexerContext const context) {
  LexerContext copy;
  copy.currentChar = context.currentChar;
  // TODO: not necessary
  copy.currentToken = copyToken(context.currentToken);
  copy.allocator = createBlockAllocator(4096);
  copy.code = context.code;
  copy.keywords = context.keywords;
  copy.line = context.line;
  copy.pos = context.pos;

  advanceToken(copy);

  // TODO: note anything using this block allocator will die with it
  dropBlockAllocator(copy.allocator);

  return copyToken(copy.currentToken);
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
      case '$':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::DOLLAR, nullptr);
      case '/':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::DIVIDE, nullptr);
      case '=':
        advanceCharacter(context);
        return createToken(context.allocator, TokenType::EQUAL, nullptr);
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
  uint64_t index = context.pos + 1;

  while (index < context.code.length() &&
         (std::isspace(context.code[index]) || context.code[index] == '\n')) {
    ++index;
  }

  if (index < context.code.length()) {
    return context.code[index];
  }

  return '\0';
}

void expectToken(LexerContext& context, TokenType type) {
  VIVIUM_ASSERT(context.currentToken.type == type,
                "Invalid token, expected type {} but got {}",
                tokenTypeString(type),
                tokenTypeString(context.currentToken.type));

  VIVIUM_LOG(LogSeverity::DEBUG, "Expected token of type {} correctly",
             tokenTypeString(context.currentToken.type));

  advanceToken(context);
}

Token createToken(BlockAllocator& allocator, TokenType type, void const* data) {
  Token token;
  token.type = type;

  VIVIUM_LOG(LogSeverity::DEBUG, "Creating token of type {}, data at {}",
             tokenTypeString(type), data);

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
    case TokenType::FOR:
    case TokenType::WHILE:
    case TokenType::IF:
    case TokenType::AS:
    case TokenType::ELSE:
    case TokenType::SEMICOLON:
    case TokenType::UNKNOWN:
    case TokenType::COMMA:
    case TokenType::OVERRIDE:
    case TokenType::FUNCTION:
    case TokenType::CLASS:
    case TokenType::DOLLAR:
      break;
    case TokenType::INTEGER: {
      new (token.memory.data())
          TokenInteger(*reinterpret_cast<int64_t const*>(data));
      break;
    }
    case TokenType::FLOATING: {
      new (token.memory.data())
          TokenFloating(*reinterpret_cast<double const*>(data));
      break;
    }
    case TokenType::IDENTIFIER: {
      TokenIdentifier* identifier = new (token.memory.data()) TokenIdentifier();
      *identifier =
          createTokenString(*reinterpret_cast<std::string const*>(data));

      VIVIUM_LOG(LogSeverity::DEBUG, "Token memory created, now reads: [{}]",
                 getTokenString(
                     *reinterpret_cast<TokenIdentifier*>(token.memory.data())));

      break;
    }
  }

  return token;
}

void destroyToken(Token& token) {
  VIVIUM_LOG(LogSeverity::DEBUG, "Destroying token of type {}",
             tokenTypeString(token.type));

  switch (token.type) {
    case TokenType::INTEGER:
      reinterpret_cast<TokenInteger*>(token.memory.data())->~TokenInteger();
      break;
    case TokenType::FLOATING:
      reinterpret_cast<TokenFloating*>(token.memory.data())->~TokenFloating();
      break;
    case TokenType::IDENTIFIER:
      dropTokenString(*reinterpret_cast<TokenIdentifier*>(token.memory.data()));
      reinterpret_cast<TokenIdentifier*>(token.memory.data())
          ->~TokenIdentifier();
      break;
    default:
      break;
  }
}
}  // namespace GUIF
}  // namespace Vivium

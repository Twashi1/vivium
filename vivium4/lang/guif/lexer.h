#pragma once

#include <array>
#include <cctype>
#include <unordered_map>

#include "../../storage.h"

namespace Vivium {
namespace GUIF {
inline constexpr uint64_t TOKEN_BLOCK_SIZE = 4096;

enum class TokenType : uint16_t {
  UNKNOWN,
  IDENTIFIER,
  INTEGER,
  FLOATING,
  COLON,
  COMMA,
  SEMICOLON,
  EQUAL,
  CURLY_LEFT,
  CURLY_RIGHT,
  PAREN_LEFT,
  PAREN_RIGHT,
  SQUARE_LEFT,
  SQUARE_RIGHT,
  DOLLAR,
  PLUS,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  IF,
  ELSE,
  AS,
  WHILE,
  FOR,
  CLASS,
  FUNCTION,
  OVERRIDE,
  END_OF_FILE
};

struct TokenString {
  char* address;
  uint64_t size;
  std::array<uint8_t, 16> sso;
};

TokenString createTokenString(std::string_view string);
void dropTokenString(TokenString const& string);
TokenString copyTokenString(TokenString const& string);
std::string getTokenString(TokenString const& string);

using TokenIdentifier = TokenString;
using TokenInteger = int64_t;
using TokenFloating = double;

union TokenMemory {
  TokenIdentifier identifier;
  TokenInteger integer;
  TokenFloating floating;
};

struct Token {
  TokenType type;
  alignas(TokenMemory) std::array<uint8_t, sizeof(TokenMemory)> memory;
};

struct LexerContext {
  std::string_view code;
  int pos;
  int line;
  char currentChar;
  BlockAllocator allocator;
  Token currentToken;
  std::unordered_map<std::string, TokenType> keywords;
};

LexerContext createLexer(std::string_view text);
void dropLexer(LexerContext& context);

std::string tokenTypeString(TokenType type);
Token _getNextToken(LexerContext& context);
void advanceToken(LexerContext& context);
// NOTE: very expensive method
Token peekToken(LexerContext const context);
char advanceCharacter(LexerContext& context);
char peekCharacter(LexerContext const& context);

Token readNumber(LexerContext& context);
Token readIdentifier(LexerContext& context);

void expectToken(LexerContext& context, TokenType type);
Token copyToken(Token const token);

// Assumes data to be move-able
Token createToken(BlockAllocator& allocator, TokenType type, void const* data);
void destroyToken(Token& token);
}  // namespace GUIF
}  // namespace Vivium

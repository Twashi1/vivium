#pragma once

#include <cctype>
#include <unordered_map>

#include "../../storage.h"

namespace Vivium {
namespace GUIF {
inline constexpr uint64_t TOKEN_BLOCK_SIZE = 4096;

enum class TokenType {
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
  PLUS,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  IF,
  ELSE,
  AS,
  WHILE,
  FOR,
  END_OF_FILE
};

constexpr uint64_t inplaceTokenSize =
    std::max({sizeof(uint64_t), sizeof(double), sizeof(std::string)});

struct Token {
  TokenType type;
  union {
    void* address;
    uint8_t inplace[inplaceTokenSize];
  };
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
Token advanceToken(LexerContext& context);
char advanceCharacter(LexerContext& context);
char peekCharacter(LexerContext const& context);

Token readNumber(LexerContext& context);
Token readIdentifier(LexerContext& context);

Token expectToken(LexerContext& context, TokenType type);
Token copyToken(BlockAllocator& allocator, Token const token);

// Assumes data to be move-able
Token createToken(BlockAllocator& allocator, TokenType type, void const* data);
}  // namespace GUIF
}  // namespace Vivium

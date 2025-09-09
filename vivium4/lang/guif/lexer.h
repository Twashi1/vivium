#pragma once

#include <cctype>

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
  DIVIDE
};

constexpr uint64_t inplaceTokenSize =
    std::max({sizeof(uint64_t), sizeof(double), sizeof(std::string)});

struct Token {
  TokenType type;
  union {
    std::byte* address;
    std::byte inplace[inplaceTokenSize];
  };
};

struct LexerContext {
  std::string_view code;
  int pos;
  int line;
  char currentChar;
  BlockAllocator allocator;
  Token currentToken;
};

LexerContext createLexer(std::string_view text);
void dropLexer(LexerContext& context);

Token advanceToken(LexerContext& context);
char advanceCharacter(LexerContext& context);
char peekCharacter(LexerContext const& context);

Token readNumber(LexerContext& context);
Token readIdentifier(LexerContext& context);

Token expectToken(LexerContext& context, TokenType type);

// Assumes data to be move-able
Token createToken(TokenType type, void* data);
}  // namespace GUIF
}  // namespace Vivium

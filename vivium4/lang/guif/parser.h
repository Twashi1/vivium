#pragma once

#include "ast.h"
#include "lexer.h"

namespace Vivium {
namespace GUIF {
struct ParserContext {
  LexerContext lexer;
  ASTContext astContext;
};

// Recursive descent parser
// All multiplication/division occurs first
// Then all addition/subtraction
// So when we see add/sub, we split the expression to evaluate the left and then
// the right, and then add/sub the result
//  if we don't see add/sub, we recurse down a level

// Higher precedence implies the operation is performed first
enum class PrecedenceOrder : uint32_t { VALUE, MUL_DIV, ADD_SUB, EXPRESSION };

ParserContext createParserContext(LexerContext lexer, ASTContext ast);
// TODO: ambiguity whether parser context should be free-ing its members
void dropParserContext(ParserContext& parser);

PrecedenceOrder _advancePrecedence(PrecedenceOrder order);
ASTNode _parseValue(ParserContext& context);
ASTNode _parseExpression(ParserContext& context, PrecedenceOrder order);
ASTNode _parseAddSub(ParserContext& context, ASTNode left);
ASTNode _parseMulDiv(ParserContext& context, ASTNode left);
ASTNode parse(ParserContext& context);
}  // namespace GUIF
}  // namespace Vivium

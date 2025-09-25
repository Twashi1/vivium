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

//  TODO: parenthesis

// Higher precedence implies the operation is performed first
enum class PrecedenceOrder : uint32_t {
  VALUE,
  FUNCTION_CALL,
  MUL_DIV,
  ADD_SUB,
  // AS,
  ASSIGN,
  EXPRESSION
};

ParserContext createParserContext(LexerContext lexer, ASTContext ast);
// TODO: ambiguity whether parser context should be free-ing its members
void dropParserContext(ParserContext& parser);

PrecedenceOrder _advancePrecedence(PrecedenceOrder order);
ASTNode _parseValue(ParserContext& context);
ASTNode _parseExpression(ParserContext& context, PrecedenceOrder order);
ASTNode _parseAddSub(ParserContext& context, ASTNode left);
ASTNode _parseMulDiv(ParserContext& context, ASTNode left);
ASTNode _parseFunctionCall(ParserContext& context, ASTNode left);
ASTNode _parseAssignment(ParserContext& context, ASTNode name);
ASTNode _parseAs(ParserContext& context, ASTNode left);
ASTNode _parseFunctionDefinition(ParserContext& context);
ASTNode _parseCompoundStatement(ParserContext& context);
ASTNode _parseStatement(ParserContext& context);
ASTNode _parseStatementIdentifier(ParserContext& context,
                                  Token const lastIdentifier);
ASTNode parse(ParserContext& context);
}  // namespace GUIF
}  // namespace Vivium

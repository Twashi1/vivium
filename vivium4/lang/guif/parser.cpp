#include "parser.h"

#include "ast.h"

namespace Vivium {
namespace GUIF {
ParserContext createParserContext(LexerContext lexer, ASTContext ast) {
  ParserContext parser;
  parser.lexer = lexer;
  parser.astContext = ast;

  return parser;
}

void dropParserContext(ParserContext& parser) {}

ASTNode parse(ParserContext& context) {
  advanceToken(context.lexer);

  ASTNode root = _parseCompoundStatement(context);

  return root;
}

PrecedenceOrder _advancePrecedence(PrecedenceOrder order) {
  return static_cast<PrecedenceOrder>(static_cast<uint32_t>(order) - 1);
}

ASTNode _parseExpression(ParserContext& context, PrecedenceOrder order) {
  // Recursive base case
  if (order == PrecedenceOrder::VALUE) {
    return _parseValue(context);
  }

  ASTNode lhs = _parseExpression(context, _advancePrecedence(order));

  if (context.lexer.currentToken.type == TokenType::END_OF_FILE) {
    return lhs;
  }

  // TODO: we get stuck in an infinite loop here
  //  if we do not find the current token is applicable to this
  //  precedence level, we should return
  switch (order) {
    case PrecedenceOrder::VALUE:
      break;  // This arm is never reached
    case PrecedenceOrder::ASSIGN:
      return _parseAssignment(context, lhs);
    case PrecedenceOrder::ADD_SUB:
      return _parseAddSub(context, lhs);
    case PrecedenceOrder::MUL_DIV:
      return _parseMulDiv(context, lhs);
    case PrecedenceOrder::EXPRESSION:
      return lhs;
    default:
      VIVIUM_LOG(LogSeverity::FATAL, "Invalid precedence value of {}",
                 static_cast<uint32_t>(order));
      break;
  }

  VIVIUM_LOG(LogSeverity::ERROR,
             "Unknown node or precedence value encountered");
  return createASTNode(context.astContext.allocator, ASTNodeType::UNKNOWN,
                       nullptr);
}

ASTNode _parseValue(ParserContext& context) {
  switch (context.lexer.currentToken.type) {
    case TokenType::FLOATING:
    case TokenType::INTEGER:
      NodeNumber number;
      number.value = context.lexer.currentToken;
      advanceToken(context.lexer);

      return createASTNode(context.astContext.allocator, ASTNodeType::NUMBER,
                           &number);
    case TokenType::PAREN_LEFT: {
      expectToken(context.lexer, TokenType::PAREN_LEFT);
      ASTNode inner = _parseExpression(context, PrecedenceOrder::EXPRESSION);
      expectToken(context.lexer, TokenType::PAREN_RIGHT);

      return inner;
    }
    case TokenType::IDENTIFIER: {
      NodeVar variable;
      variable.name =
          *reinterpret_cast<std::string*>(context.lexer.currentToken.inplace);
      advanceToken(context.lexer);

      return createASTNode(context.astContext.allocator, ASTNodeType::VAR,
                           &variable);
    }
    default:
      VIVIUM_LOG(LogSeverity::ERROR, "Expected a value, but got token {}",
                 tokenTypeString(context.lexer.currentToken.type));
      break;
  }

  return createASTNode(context.astContext.allocator, ASTNodeType::UNKNOWN,
                       nullptr);
}

ASTNode _parseAddSub(ParserContext& context, ASTNode left) {
  while (context.lexer.currentToken.type != TokenType::END_OF_FILE) {
    switch (context.lexer.currentToken.type) {
      case TokenType::PLUS:
      case TokenType::SUBTRACT: {
        NodeBinaryOp binaryOp;
        binaryOp.left = left;

        ASTNodeType nodeType =
            context.lexer.currentToken.type == TokenType::PLUS
                ? ASTNodeType::ADD_OP
                : ASTNodeType::SUB_OP;

        advanceToken(context.lexer);
        binaryOp.right = _parseExpression(
            context, _advancePrecedence(PrecedenceOrder::ADD_SUB));

        left = createASTNode(context.astContext.allocator, nodeType, &binaryOp);
        break;
      }
      default:
        return left;
    }
  }
  return left;
}
ASTNode _parseAssignment(ParserContext& context, ASTNode left) {
  if (context.lexer.currentToken.type != TokenType::EQUAL) {
    return left;
  }

  VIVIUM_ASSERT(left.type == ASTNodeType::VAR,
                "Expected variable on left of assignment");

  expectToken(context.lexer, TokenType::EQUAL);

  ASTNode right = _parseExpression(context, PrecedenceOrder::EXPRESSION);

  NodeBinaryOp assign;
  assign.left = left;
  assign.right = right;

  return createASTNode(context.astContext.allocator, ASTNodeType::ASSIGN_OP,
                       &assign);
}

ASTNode _parseMulDiv(ParserContext& context, ASTNode left) {
  while (context.lexer.currentToken.type != TokenType::END_OF_FILE) {
    switch (context.lexer.currentToken.type) {
      case TokenType::MULTIPLY:
      case TokenType::DIVIDE: {
        NodeBinaryOp binaryOp;
        binaryOp.left = left;

        ASTNodeType nodeType =
            context.lexer.currentToken.type == TokenType::MULTIPLY
                ? ASTNodeType::MUL_OP
                : ASTNodeType::DIV_OP;

        advanceToken(context.lexer);
        binaryOp.right = _parseExpression(
            context, _advancePrecedence(PrecedenceOrder::MUL_DIV));

        left = createASTNode(context.astContext.allocator, nodeType, &binaryOp);
        break;
      }
      default:
        return left;
    }
  }

  return left;
}

ASTNode _parseCompoundStatement(ParserContext& context) {
  expectToken(context.lexer, TokenType::CURLY_LEFT);

  std::vector<ASTNode> statements;

  while (context.lexer.currentToken.type != TokenType::CURLY_RIGHT) {
    VIVIUM_LOG(LogSeverity::DEBUG, "Grabbing statement...");

    ASTNode statement = _parseStatement(context);
    statements.push_back(statement);
    // TODO: maybe should be inside the parse statement?
    expectToken(context.lexer, TokenType::SEMICOLON);
  }

  expectToken(context.lexer, TokenType::CURLY_RIGHT);

  NodeCompound compound;
  compound.nodes = statements;

  return createASTNode(context.astContext.allocator, ASTNodeType::COMPOUND,
                       &compound);
}

ASTNode _parseStatement(ParserContext& context) {
  switch (context.lexer.currentToken.type) {
    // function call or assignment
    case TokenType::IDENTIFIER: {
      Token identifier =
          copyToken(context.lexer.allocator, context.lexer.currentToken);

      return _parseStatementIdentifier(context, identifier);
    }
    default:
      return _parseExpression(context, PrecedenceOrder::EXPRESSION);
  }
}

ASTNode _parseStatementIdentifier(ParserContext& context,
                                  Token lastIdentifier) {
  expectToken(context.lexer, TokenType::IDENTIFIER);

  switch (context.lexer.currentToken.type) {
    case TokenType::PAREN_LEFT:
      return _parseFunctionCall(context, lastIdentifier);
    case TokenType::EQUAL: {
      NodeVar left;
      left.name = *reinterpret_cast<std::string*>(lastIdentifier.inplace);
      ASTNode leftVar =
          createASTNode(context.astContext.allocator, ASTNodeType::VAR, &left);

      return _parseAssignment(context, leftVar);
    }
    default:
      VIVIUM_LOG(LogSeverity::ERROR,
                 "Malformed expression, expected function call or assignment "
                 "after identifier, instead had {}",
                 tokenTypeString(context.lexer.currentToken.type));
      return createASTNode(context.astContext.allocator, ASTNodeType::UNKNOWN,
                           nullptr);
  }
}

ASTNode _parseFunctionCall(ParserContext& context, Token name) {
  expectToken(context.lexer, TokenType::PAREN_LEFT);

  NodeFunctionCall call;
  call.name = *reinterpret_cast<std::string*>(name.inplace);
  // TODO: fill out and parse other parameters

  expectToken(context.lexer, TokenType::PAREN_RIGHT);

  return createASTNode(context.astContext.allocator, ASTNodeType::FUNCTION_CALL,
                       &call);
}
}  // namespace GUIF
}  // namespace Vivium

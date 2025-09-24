#include "interpreter.h"

#include "ast.h"
#include "interpreter_impl.h"

namespace Vivium {
namespace GUIF {
ASTNode evaluate(InterpreterContext& context, ASTNode root) {
  switch (root.type) {
    case ASTNodeType::ADD_OP:
      return visitAddOp(context, reinterpret_cast<NodeBinaryOp*>(root.data));
    case ASTNodeType::SUB_OP:
      return visitSubOp(context, reinterpret_cast<NodeBinaryOp*>(root.data));
    case ASTNodeType::MUL_OP:
      return visitMulOp(context, reinterpret_cast<NodeBinaryOp*>(root.data));
    case ASTNodeType::DIV_OP:
      return visitDivOp(context, reinterpret_cast<NodeBinaryOp*>(root.data));
    case ASTNodeType::ASSIGN_OP:
      return visitAssignOp(context, reinterpret_cast<NodeBinaryOp*>(root.data));
    case ASTNodeType::COMPOUND:
      return visitCompound(context, reinterpret_cast<NodeCompound*>(root.data));
    case ASTNodeType::VAR:
      return visitVar(context, reinterpret_cast<NodeVar*>(root.data));
    case ASTNodeType::NUMBER:
      return root;
    case ASTNodeType::UNKNOWN:
      // TODO: throw error?
      return root;
    default:
      VIVIUM_LOG(LogSeverity::ERROR, "Invalid token type to visit, value: {}",
                 static_cast<uint32_t>(root.type));

      break;
  }

  return createASTNode(context.allocator, ASTNodeType::UNKNOWN, nullptr);
}

std::string _symbolString(FunctionSymbol symbol) {
  std::ostringstream ss;
  for (TypeSymbol const& typeSymbol : symbol.parameters) {
    ss << _symbolString(typeSymbol) << ',';
  }

  return std::format("{} {}({})", _symbolString(symbol.returnType),
                     symbol.functionName, ss.str());
}
std::string _symbolString(TypeSymbol symbol) { return symbol.name; }
std::string _functionSymbolToKey(FunctionSymbol symbol) {
  std::ostringstream ss;
  for (TypeSymbol const& typeSymbol : symbol.parameters) {
    ss << _symbolString(typeSymbol) << ',';
  }

  return std::format("{}({})", symbol.functionName, ss.str());
}
std::vector<TypeSymbol> _tokensToTypeSymbols(
    std::vector<Token> const& parameters) {
  std::vector<TypeSymbol> symbols = std::vector<TypeSymbol>(parameters.size());

  for (uint64_t i = 0; i < parameters.size(); i++) {
    symbols[i] = _tokenToTypeSymbol(parameters[i]);
  }

  return symbols;
}
TypeSymbol _tokenToTypeSymbol(Token const& token) {
  switch (token.type) {
    case TokenType::INTEGER:
      return TypeSymbol("i64");
    case TokenType::FLOATING:
      return TypeSymbol("f64");
    default:
      VIVIUM_LOG(LogSeverity::ERROR,
                 "Invalid token type to convert to type symbol: {}",
                 static_cast<uint64_t>(token.type));
      return TypeSymbol("UNKNOWN");
  }
}

LanguageFunction lookupFunction(InterpreterContext& context,
                                std::string_view functionName,
                                std::vector<TypeSymbol> const& parameters) {
  // TODO: requires construction of a dummy function symbol for lookup
  //  (bad)
  FunctionSymbol fakeSymbol;
  fakeSymbol.functionName = functionName;
  fakeSymbol.parameters = parameters;

  std::string functionKey = _functionSymbolToKey(fakeSymbol);

  if (context.functionMap.contains(functionKey)) {
    return context.functionMap.at(functionKey);
  }

  // TODO: lookup every common type cast of the parameters
  std::ostringstream ss;
  for (auto const& it : context.functionMap) {
    std::string const& name = it.first;
    LanguageFunction const& ptr = it.second;

    ss << std::format("[{}: {}]\n", name, reinterpret_cast<void const*>(ptr));
  }

  VIVIUM_LOG(LogSeverity::DEBUG, "Function map table: {}", ss.str());
  VIVIUM_LOG(LogSeverity::ERROR, "Couldn't find function with symbol {}",
             functionKey);

  return nullptr;
}

ASTNode visitAddOp(InterpreterContext& context, NodeBinaryOp* binary) {
  return visitBinaryOp(context, binary, "__add");
}

ASTNode visitSubOp(InterpreterContext& context, NodeBinaryOp* binary) {
  return visitBinaryOp(context, binary, "__sub");
}
ASTNode visitMulOp(InterpreterContext& context, NodeBinaryOp* binary) {
  return visitBinaryOp(context, binary, "__mul");
}
ASTNode visitDivOp(InterpreterContext& context, NodeBinaryOp* binary) {
  return visitBinaryOp(context, binary, "__div");
}
ASTNode visitAssignOp(InterpreterContext& context, NodeBinaryOp* binary) {
  VIVIUM_ASSERT(binary->left.type == ASTNodeType::VAR,
                "Expected variable on left of assignment");

  NodeVar* var = reinterpret_cast<NodeVar*>(binary->left.data);
  ASTNode right = evaluate(context, binary->right);

  if (context.variableMap.contains(var->name)) {
    context.variableMap.at(var->name) = right;
  } else {
    context.variableMap.insert({var->name, right});
  }

  return copyNode(context, right);
}

ASTNode visitBinaryOp(InterpreterContext& context, NodeBinaryOp* binary,
                      std::string_view functionName) {
  ASTNode left = evaluate(context, binary->left);
  ASTNode right = evaluate(context, binary->right);

  // Expect left and right to be numerical types
  VIVIUM_ASSERT(left.type == ASTNodeType::NUMBER,
                "Expected left of add to resolve number");
  VIVIUM_ASSERT(right.type == ASTNodeType::NUMBER,
                "Expected right of add to resolve to number");

  NodeNumber* leftNum = reinterpret_cast<NodeNumber*>(left.data);
  NodeNumber* rightNum = reinterpret_cast<NodeNumber*>(right.data);

  std::vector<Token> parameters =
      std::vector<Token>({leftNum->value, rightNum->value});

  std::vector<TypeSymbol> parameterTypes = _tokensToTypeSymbols(parameters);

  LanguageFunction function =
      lookupFunction(context, functionName, parameterTypes);
  // Assuming we take ownership here, so no need to copy
  Token result = function(context, parameters);

  // TODO: assumes result is a number
  return createASTNode(context.allocator, ASTNodeType::NUMBER, &result);
}

ASTNode visitVar(InterpreterContext& context, NodeVar* var) {
  if (!context.variableMap.contains(var->name)) {
    VIVIUM_LOG(LogSeverity::ERROR, "Couldn't find variable {}", var->name);

    return createASTNode(context.allocator, ASTNodeType::UNKNOWN, nullptr);
  }

  return copyNode(context, context.variableMap.at(var->name));
}

ASTNode visitCompound(InterpreterContext& context, NodeCompound* compound) {
  // TODO: null ast node value?
  ASTNode lastResult =
      createASTNode(context.allocator, ASTNodeType::UNKNOWN, nullptr);

  for (ASTNode child : compound->nodes) {
    lastResult = evaluate(context, child);
  }

  return lastResult;
}

ASTNode copyNode(InterpreterContext& context, ASTNode node) {
  switch (node.type) {
    case ASTNodeType::ADD_OP:
    case ASTNodeType::SUB_OP:
    case ASTNodeType::MUL_OP:
    case ASTNodeType::DIV_OP:
    case ASTNodeType::ASSIGN_OP: {
      NodeBinaryOp operationCopy;
      NodeBinaryOp* operation = reinterpret_cast<NodeBinaryOp*>(node.data);
      operationCopy.left = copyNode(context, operation->left);
      operationCopy.right = copyNode(context, operation->right);

      return createASTNode(context.allocator, node.type, &operationCopy);
    }
    // TODO: incorrect handling of these cases
    case ASTNodeType::FUNCTION_DEFINITION:
    case ASTNodeType::FUNCTION_CALL:
    case ASTNodeType::COMPOUND:
    case ASTNodeType::UNKNOWN:
      return node;
    case ASTNodeType::VAR: {
      NodeVar* var = reinterpret_cast<NodeVar*>(node.data);
      NodeVar varCopy;
      varCopy.name = var->name;

      return createASTNode(context.allocator, node.type, &varCopy);
    }
    case ASTNodeType::NUMBER: {
      NodeNumber* number = reinterpret_cast<NodeNumber*>(node.data);
      NodeNumber numberCopy;
      numberCopy.value = copyToken(number->value);

      return createASTNode(context.allocator, node.type, &numberCopy);
    }
  }
}

Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    int64_t const value) {
  return createToken(allocator, TokenType::INTEGER, &value);
}
Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    double const value) {
  return createToken(allocator, TokenType::FLOATING, &value);
}

template <>
std::string_view binaryOpTypeString<BinaryOpType::ADD>() {
  return "__add";
}
template <>
std::string_view binaryOpTypeString<BinaryOpType::SUB>() {
  return "__sub";
}
template <>
std::string_view binaryOpTypeString<BinaryOpType::MUL>() {
  return "__mul";
}
template <>
std::string_view binaryOpTypeString<BinaryOpType::DIV>() {
  return "__div";
}

// TODO: more template metaprogramming bs to automate this to a higher degree,
//  i.e. just listing the types uint64_t, double, etc., and listing the binary
//  op types and it iterates all combinations
void _registerBuiltinFunctions(InterpreterContext& context) {
  _registerBinaryFunction<int64_t, BinaryOpType::ADD>(context);
  _registerBinaryFunction<double, BinaryOpType::ADD>(context);

  _registerBinaryFunction<int64_t, BinaryOpType::SUB>(context);
  _registerBinaryFunction<double, BinaryOpType::SUB>(context);

  _registerBinaryFunction<int64_t, BinaryOpType::MUL>(context);
  _registerBinaryFunction<double, BinaryOpType::MUL>(context);

  _registerBinaryFunction<int64_t, BinaryOpType::DIV>(context);
  _registerBinaryFunction<double, BinaryOpType::DIV>(context);
}

void _registerFunction(InterpreterContext& context, FunctionSymbol symbol,
                       LanguageFunction function) {
  context.functionMap.insert({_functionSymbolToKey(symbol), function});
}

InterpreterContext createInterpreter(uint64_t blockSize) {
  InterpreterContext context;
  context.allocator = createBlockAllocator(blockSize);

  _registerBuiltinFunctions(context);

  return context;
}

void dropInterpreter(InterpreterContext& context) {
  dropBlockAllocator(context.allocator);
}

std::string printTree(ASTNode root) {
  switch (root.type) {
    case ASTNodeType::ADD_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(root.data);
      return std::format("[ADD_OP {} {}]", printTree(binary->left),
                         printTree(binary->right));
    }
    case ASTNodeType::SUB_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(root.data);
      return std::format("[SUB_OP {} {}]", printTree(binary->left),
                         printTree(binary->right));
    }
    case ASTNodeType::MUL_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(root.data);
      return std::format("[MUL_OP {} {}]", printTree(binary->left),
                         printTree(binary->right));
    }
    case ASTNodeType::DIV_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(root.data);
      return std::format("[DIV_OP {} {}]", printTree(binary->left),
                         printTree(binary->right));
    }
    case ASTNodeType::ASSIGN_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(root.data);
      return std::format("[ASSIGN_OP {} {}]", printTree(binary->left),
                         printTree(binary->right));
    }
    case ASTNodeType::VAR: {
      NodeVar* var = reinterpret_cast<NodeVar*>(root.data);
      // TODO: print value from context as well
      return std::format("[VAR {} VALUE?]", var->name);
    }
    case ASTNodeType::NUMBER: {
      NodeNumber* number = reinterpret_cast<NodeNumber*>(root.data);

      switch (number->value.type) {
        case TokenType::INTEGER:
          return std::format(
              "{}", *reinterpret_cast<int64_t*>(number->value.memory.data()));
        case TokenType::FLOATING:
          return std::format(
              "{}", *reinterpret_cast<double*>(number->value.memory.data()));
        default:
          break;
      }
    }

      return "INVALID_NUMBER";
    case ASTNodeType::FUNCTION_CALL:
      // TODO: more printing
      return "FUNCTION_CALL";
    case ASTNodeType::FUNCTION_DEFINITION:
      // TODO: more printign
      return "FUNCTION_DEFINITION";
    case ASTNodeType::COMPOUND: {
      NodeCompound* compound = reinterpret_cast<NodeCompound*>(root.data);

      std::ostringstream ss;

      for (ASTNode child : compound->nodes) {
        ss << printTree(child) << ",";
      }

      return std::format("[COMPOUND {}]", ss.str());
    }
    case ASTNodeType::UNKNOWN:
      return "UNKNOWN";
    default:
      return std::format("Unhandled node type with value {}",
                         static_cast<uint64_t>(root.type));
  }
}
}  // namespace GUIF
}  // namespace Vivium

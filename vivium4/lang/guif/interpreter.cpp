#include "interpreter.h"

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
    case ASTNodeType::VAR:
      // TODO: should look up the value and return it?
      return root;
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
  // TODO: implementation
  return createASTNode(context.allocator, ASTNodeType::UNKNOWN, nullptr);
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

  NodeNumber* leftNum = reinterpret_cast<NodeNumber*>(left.inplace);
  NodeNumber* rightNum = reinterpret_cast<NodeNumber*>(right.inplace);

  std::vector<Token> parameters =
      std::vector<Token>({leftNum->value, rightNum->value});

  std::vector<TypeSymbol> parameterTypes = _tokensToTypeSymbols(parameters);

  LanguageFunction function =
      lookupFunction(context, functionName, parameterTypes);
  Token result = function(context, parameters);

  // TODO: assumes result is a number
  return createASTNode(context.allocator, ASTNodeType::NUMBER, &result);
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
      NodeVar* var = reinterpret_cast<NodeVar*>(root.inplace);
      // TODO: print value from context as well
      return std::format("[VAR {} VALUE?]", var->name);
    }
    case ASTNodeType::NUMBER:
      // TODO: unsure on the usage here
      {
        NodeNumber* number = reinterpret_cast<NodeNumber*>(root.inplace);

        switch (number->value.type) {
          case TokenType::INTEGER:
            return std::format(
                "{}", *reinterpret_cast<int64_t*>(number->value.inplace));
          case TokenType::FLOATING:
            return std::format(
                "{}", *reinterpret_cast<double*>(number->value.inplace));
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
    case ASTNodeType::UNKNOWN:
    default:
      return "Unknown";
  }
}
}  // namespace GUIF
}  // namespace Vivium

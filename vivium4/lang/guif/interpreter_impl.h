#include "interpreter.h"

namespace Vivium {
namespace GUIF {
template <typename T, BinaryOpType operation>
Token executeBinaryOp(InterpreterContext& interp,
                      std::vector<Token> const& tokens) {
  T const& left = *reinterpret_cast<T const*>(tokens[0].memory.data());
  T const& right = *reinterpret_cast<T const*>(tokens[1].memory.data());
  T result;

  if constexpr (operation == BinaryOpType::ADD) {
    result = left + right;
  } else if constexpr (operation == BinaryOpType::SUB) {
    result = left - right;
  } else if constexpr (operation == BinaryOpType::MUL) {
    result = left * right;
  } else if constexpr (operation == BinaryOpType::DIV) {
    result = left / right;
  } else {
    static_assert(
        false &&
        "Failed to specialise template binary op, unimplemented operation "
        "type");
  }

  return _createTokenFromNumericalType(interp.allocator, result);
}
template <typename T>
TypeSymbol _makeTypeSymbolBuiltin() {
  if constexpr (std::is_same_v<T, int64_t>) {
    return TypeSymbol("i64");
  } else if constexpr (std::is_same_v<T, double>) {
    return TypeSymbol("f64");
  }

  VIVIUM_LOG(LogSeverity::WARN,
             "Defaulting to pretty type name for builtin type {}",
             prettyTypeName<T>());
  return TypeSymbol(prettyTypeName<T>());
}

template <typename T, BinaryOpType operation>
FunctionSymbol _makeSymbolBinaryBuiltin() {
  FunctionSymbol symbol;
  symbol.functionName = binaryOpTypeString<operation>();
  symbol.returnType = _makeTypeSymbolBuiltin<T>();
  symbol.parameters = std::vector<TypeSymbol>(
      {_makeTypeSymbolBuiltin<T>(), _makeTypeSymbolBuiltin<T>()});

  return symbol;
}
template <typename T, BinaryOpType operation>
void _registerBinaryFunction(InterpreterContext& context) {
  _registerFunction(context, _makeSymbolBinaryBuiltin<T, operation>(),
                    executeBinaryOp<T, operation>);
}
}  // namespace GUIF
}  // namespace Vivium

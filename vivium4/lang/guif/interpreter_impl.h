#include "interpreter.h"

namespace Vivium {
namespace GUIF {
template <>
struct PreferredType<uint64_t, uint64_t> {
  using type = uint64_t;
};
template <>
struct PreferredType<double, uint64_t> {
  using type = double;
};
template <>
struct PreferredType<double, double> {
  using type = double;
};
template <>
struct PreferredType<uint64_t, double> {
  using type = double;
};

template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _add(LeftType const left,
                                                       LeftType const right) {
  return left + right;
}
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _sub(LeftType const left,
                                                       LeftType const right) {
  return left - right;
}
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _mul(LeftType const left,
                                                       LeftType const right) {
  return left * right;
}
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type::type _div(
    LeftType const left, LeftType const right) {
  return left / right;
}

// TODO: the lexer context is a bit weird here, we need memory
//  so we use lexer, but its nicer to attach it to interpreter memory
//  or something similar
template <typename LeftType, typename RightType>
Token performBinaryOp(InterpreterContext& context, Token const& left,
                      Token const& right,
                      BinaryOperationFunction<LeftType, RightType> function) {
  typename PreferredType<LeftType, RightType>::type result =
      function(*reinterpret_cast<LeftType const*>(left.inplace),
               *reinterpret_cast<RightType const*>(right.inplace));

  return _createTokenFromNumericalType(context.allocator, result);
}

template <typename LeftType, typename RightType, BinaryOpType operation>
Token templateBinaryOp(InterpreterContext& interp,
                       std::vector<Token> const& tokens) {
  if constexpr (operation == BinaryOpType::ADD)
    return performBinaryOp(interp, tokens[0], tokens[1],
                           _add<LeftType, RightType>);
  else if constexpr (operation == BinaryOpType::SUB)
    return performBinaryOp(interp, tokens[0], tokens[1],
                           _sub<LeftType, RightType>);
  else if constexpr (operation == BinaryOpType::MUL)
    return performBinaryOp(interp, tokens[0], tokens[1],
                           _mul<LeftType, RightType>);
  else if constexpr (operation == BinaryOpType::DIV)
    return performBinaryOp(interp, tokens[0], tokens[1],
                           _div<LeftType, RightType>);
  else
    static_assert(
        false &&
        "Failed to specialise template binary op, unimplemented operation "
        "type");
}
template <typename T>
TypeSymbol _makeTypeSymbolBuiltin() {
  return TypeSymbol(prettyTypeName<T>());
}
template <BinaryOpType opType>
std::string_view binaryOpTypeString() {
  return "UnimplementedBinaryOpType";
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

template <typename LeftType, typename RightType, BinaryOpType operation>
FunctionSymbol _makeSymbolBinaryBuiltin() {
  return FunctionSymbol(
      _makeTypeSymbolBuiltin<
          typename PreferredType<LeftType, RightType>::type>(),
      binaryOpTypeString<operation>(),
      std::vector<TypeSymbol>({_makeTypeSymbolBuiltin<LeftType>(),
                               _makeTypeSymbolBuiltin<RightType>()}));
}
template <typename LeftType, typename RightType, BinaryOpType operation>
void _registerBinaryFunction(InterpreterContext& context) {
  _registerFunction(context,
                    _makeSymbolBinaryBuiltin<LeftType, RightType, operation>,
                    templateBinaryOp<LeftType, RightType, operation>);
}
}  // namespace GUIF
}  // namespace Vivium

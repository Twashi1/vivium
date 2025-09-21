#pragma once

#include <unordered_map>
#include <vector>

#include "../../utility/type_names.h"
#include "ast.h"
#include "parser.h"

namespace Vivium {
namespace GUIF {

struct InterpreterContext;

using LanguageFunction = Token (*)(InterpreterContext&,
                                   std::vector<Token> const&);

// TODO: semantic analysis,
//  custom hash function
//  assuming we're movign to some sort of bytecode?
enum class BinaryOpType { ADD, SUB, MUL, DIV };

template <BinaryOpType opType>
std::string_view binaryOpTypeString();

struct TypeSymbol {
  std::string name;
};

struct FunctionSymbol {
  TypeSymbol returnType;
  std::string functionName;
  std::vector<TypeSymbol> parameters;
};

std::string _symbolString(FunctionSymbol symbol);
std::string _symbolString(TypeSymbol symbol);
std::string _functionSymbolToKey(FunctionSymbol symbol);

struct InterpreterContext {
  std::unordered_map<std::string, LanguageFunction> functionMap;
  // TODO: should be token or AST node?
  std::unordered_map<std::string, ASTNode> variableMap;
  BlockAllocator allocator;
};

template <typename LeftType, typename RightType>
struct PreferredType {
  using type = void;
};

template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _add(LeftType const left,
                                                       LeftType const right);
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _sub(LeftType const left,
                                                       LeftType const right);
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _mul(LeftType const left,
                                                       LeftType const right);
template <typename LeftType, typename RightType>
typename PreferredType<LeftType, RightType>::type _div(LeftType const left,
                                                       LeftType const right);

template <typename LeftType, typename RightType>
using BinaryOperationFunction =
    typename PreferredType<LeftType, RightType>::type (*)(LeftType const,
                                                          RightType const);

template <typename LeftType, typename RightType>
Token performBinaryOp(InterpreterContext& interp, Token const& left,
                      Token const& right,
                      BinaryOperationFunction<LeftType, RightType> function);

template <typename LeftType, typename RightType, BinaryOpType operation>
Token templateBinaryOp(InterpreterContext& interp,
                       std::vector<Token> const& tokens);

template <typename LeftType, typename RightType, BinaryOpType operation>
FunctionSymbol _makeSymbolBinaryBuiltin();
template <typename T>
TypeSymbol _makeTypeSymbolBuiltin();

// Concept on numerical type
Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    double const value);
Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    uint64_t const value);

void _registerBuiltinFunctions(InterpreterContext& context);
void _registerFunction(InterpreterContext& context, FunctionSymbol symbol,
                       LanguageFunction function);

template <typename LeftType, typename RightType, BinaryOpType operation>
void _registerBinaryFunction(InterpreterContext& context);

InterpreterContext createInterpreter(uint64_t blockSize);
void dropInterpreter(InterpreterContext& context);

// We should encode built-in functions as we do regular functions
//  so we need function-signature based lookup
//  custom hash function for functions?

ASTNode evaluate(InterpreterContext& context, ASTNode root);
ASTNode visitAddOp(InterpreterContext& context, NodeBinaryOp* binary);
ASTNode visitSubOp(InterpreterContext& context, NodeBinaryOp* binary);
ASTNode visitMulOp(InterpreterContext& context, NodeBinaryOp* binary);
ASTNode visitDivOp(InterpreterContext& context, NodeBinaryOp* binary);
ASTNode visitAssignOp(InterpreterContext& context, NodeBinaryOp* binary);
std::string printTree(ASTNode root);

}  // namespace GUIF
}  // namespace Vivium

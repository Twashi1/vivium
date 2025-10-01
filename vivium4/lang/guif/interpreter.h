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

// TODO: move all of this to AST because we need TypeSymbols
// TODO: semantic analysis,
//  custom hash function
//  assuming we're movign to some sort of bytecode?
enum class BinaryOpType { ADD, SUB, MUL, DIV };

template <BinaryOpType opType>
std::string_view binaryOpTypeString();

// TODO: generic ObjectSymbol? links to some sort of registry ig?

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

template <typename T, BinaryOpType operation>
Token executeBinaryOp(InterpreterContext& interp,
                      std::vector<Token> const& tokens);

template <typename T, BinaryOpType operation>
FunctionSymbol _makeSymbolBinaryBuiltin();
template <typename T>
TypeSymbol _makeTypeSymbolBuiltin();

// Concept on numerical type
Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    double const value);
Token _createTokenFromNumericalType(BlockAllocator& allocator,
                                    int64_t const value);

void _registerBuiltinFunctions(InterpreterContext& context);
void _registerFunction(InterpreterContext& context, FunctionSymbol symbol,
                       LanguageFunction function);

template <typename T, BinaryOpType operation>
void _registerBinaryFunction(InterpreterContext& context);
LanguageFunction lookupFunction(InterpreterContext& context,
                                std::string_view functionName,
                                std::vector<TypeSymbol> const& parameters);
std::vector<TypeSymbol> _tokensToTypeSymbols(
    std::vector<Token> const& parameters);
TypeSymbol _tokenToTypeSymbol(Token const& token);

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
ASTNode visitBinaryOp(InterpreterContext& context, NodeBinaryOp* binary,
                      std::string_view functionName);
ASTNode visitVar(InterpreterContext& context, NodeVar* var);
ASTNode visitCompound(InterpreterContext& context, NodeCompound* compound);
ASTNode copyNode(InterpreterContext& context, ASTNode node);
std::string printTree(ASTNode root);

}  // namespace GUIF
}  // namespace Vivium

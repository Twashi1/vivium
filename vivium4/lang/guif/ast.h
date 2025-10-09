#pragma once

#include <cctype>
#include <cstddef>

#include "../../storage.h"
#include "lexer.h"

namespace Vivium {
namespace GUIF {
enum class ASTNodeType {
  ADD_OP,
  SUB_OP,
  MUL_OP,
  DIV_OP,
  ASSIGN_OP,
  TYPE,
  VAR,
  NUMBER,
  FUNCTION_DEFINITION,
  FUNCTION_CALL,
  COMPOUND,
  UNKNOWN
};

struct ASTNode {
  ASTNodeType type;
  void* data;
};

struct NodeVar {
  // TODO: possible that SSO is interfering with this as well...?
  TokenString name;
};

struct NodeNumber {
  Token value;
};

struct NodeFunctionDefinition {
  std::string name;
  ASTNode returnType;
  ASTNode body;
  std::vector<ASTNode> parameters;
};

struct NodeFunctionCall {
  std::string name;
  ASTNode returnType;
  std::vector<ASTNode> arguments;
};

struct NodeBinaryOp {
  ASTNode left;
  ASTNode right;
};

struct NodeAssignOp {
  ASTNode var;
  ASTNode type;
  ASTNode right;
};

struct NodeType {
  // TypeSymbol type;
  int placeholder;
};

struct NodeCompound {
  std::vector<ASTNode> nodes;
};

struct ASTContext {
  BlockAllocator allocator;
};

ASTContext createASTContext(uint64_t blockSize);
void dropASTContext(ASTContext& context);
ASTNode createASTNode(BlockAllocator& allocator, ASTNodeType type, void* data);
void destroyASTNode(ASTNode node);
std::string nodeTypeString(ASTNodeType type);
}  // namespace GUIF
}  // namespace Vivium

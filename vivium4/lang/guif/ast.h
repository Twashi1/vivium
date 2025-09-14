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
  VAR,
  NUMBER,
  UNKNOWN
};

struct NodeNumber {
  Token value;
};

struct NodeVar {
  std::string name;
};

constexpr uint64_t _astNodeMinimumInplace =
    std::max({sizeof(NodeNumber), sizeof(NodeVar)});

struct ASTNode {
  ASTNodeType type;

  union {
    uint8_t inplace[_astNodeMinimumInplace];
    void* data;
  };
};

struct NodeBinaryOp {
  ASTNode left;
  ASTNode right;
};

struct ASTContext {
  BlockAllocator allocator;
};

ASTContext createASTContext(uint64_t blockSize);
void dropASTContext(ASTContext& context);
ASTNode createASTNode(ASTContext& context, ASTNodeType type, void* data);
std::string nodeTypeString(ASTNodeType type);
}  // namespace GUIF
}  // namespace Vivium

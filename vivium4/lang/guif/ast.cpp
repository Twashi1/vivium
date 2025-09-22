#include "ast.h"

namespace Vivium {
namespace GUIF {
ASTContext createASTContext(uint64_t blockSize) {
  ASTContext context;
  context.allocator = createBlockAllocator(blockSize);

  return context;
}

void dropASTContext(ASTContext& context) {
  dropBlockAllocator(context.allocator);
}

ASTNode createASTNode(BlockAllocator& allocator, ASTNodeType type, void* data) {
  ASTNode node;
  node.type = type;
  node.data = nullptr;

  switch (type) {
    case ASTNodeType::ADD_OP:
    case ASTNodeType::SUB_OP:
    case ASTNodeType::MUL_OP:
    case ASTNodeType::DIV_OP:
    case ASTNodeType::ASSIGN_OP:
      node.data = allocate(allocator, sizeof(NodeBinaryOp));
      new (node.data) NodeBinaryOp(*reinterpret_cast<NodeBinaryOp*>(data));
      break;
    case ASTNodeType::NUMBER:
      new (node.inplace) NodeNumber(*reinterpret_cast<NodeNumber*>(data));
      break;
    case ASTNodeType::VAR:
      new (node.inplace) NodeVar(*reinterpret_cast<NodeVar*>(data));
      break;
    case ASTNodeType::FUNCTION_DEFINITION:
      node.data = allocate(allocator, sizeof(NodeFunctionDefinition));
      new (node.data) NodeFunctionDefinition(
          *reinterpret_cast<NodeFunctionDefinition*>(data));
      break;
    case ASTNodeType::COMPOUND:
      node.data = allocate(allocator, sizeof(NodeCompound));
      new (node.data) NodeCompound(*reinterpret_cast<NodeCompound*>(data));
      break;
    case ASTNodeType::UNKNOWN:
      break;
    default:
      VIVIUM_LOG(LogSeverity::ERROR, "Unknown ASTNode type {}",
                 static_cast<uint64_t>(type));
      break;
  }

  return node;
}

std::string nodeTypeString(ASTNodeType type) {
  switch (type) {
    case ASTNodeType::ADD_OP:
      return "ADD_OP";
    case ASTNodeType::SUB_OP:
      return "SUB_OP";
    case ASTNodeType::MUL_OP:
      return "MUL_OP";
    case ASTNodeType::DIV_OP:
      return "DIV_OP";
    case ASTNodeType::ASSIGN_OP:
      return "ASSIGN_OP";
    case ASTNodeType::VAR:
      return "VAR";
    case ASTNodeType::NUMBER:
      return "NUMBER";
    case ASTNodeType::FUNCTION_DEFINITION:
      return "FUNCTION_DEFINTION";
    case ASTNodeType::FUNCTION_CALL:
      return "FUNCTION_CALL";
    case ASTNodeType::COMPOUND:
      return "COMPOUND";
    case ASTNodeType::UNKNOWN:
      return "UNKNOWN";
    default:
      return "InvalidNodeString";
  }
}
}  // namespace GUIF
}  // namespace Vivium

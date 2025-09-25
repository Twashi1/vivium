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

void destroyASTNode(ASTNode node) {
  switch (node.type) {
    case ASTNodeType::ADD_OP:
    case ASTNodeType::SUB_OP:
    case ASTNodeType::MUL_OP:
    case ASTNodeType::DIV_OP:
    case ASTNodeType::ASSIGN_OP: {
      NodeBinaryOp* binary = reinterpret_cast<NodeBinaryOp*>(node.data);
      destroyASTNode(binary->left);
      destroyASTNode(binary->right);
      binary->~NodeBinaryOp();
      break;
    }
    case ASTNodeType::VAR: {
      NodeVar* var = reinterpret_cast<NodeVar*>(node.data);
      var->~NodeVar();
      break;
    }
    case ASTNodeType::NUMBER: {
      NodeNumber* number = reinterpret_cast<NodeNumber*>(node.data);
      destroyToken(number->value);
      number->~NodeNumber();
      break;
    }
    case ASTNodeType::FUNCTION_DEFINITION: {
      NodeFunctionDefinition* definition =
          reinterpret_cast<NodeFunctionDefinition*>(node.data);
      destroyASTNode(definition->body);
      destroyASTNode(definition->returnType);

      for (ASTNode child : definition->parameters) {
        destroyASTNode(child);
      }

      definition->~NodeFunctionDefinition();
      break;
    }
    case ASTNodeType::FUNCTION_CALL: {
      NodeFunctionCall* call = reinterpret_cast<NodeFunctionCall*>(node.data);
      destroyASTNode(call->returnType);

      for (ASTNode child : call->arguments) {
        destroyASTNode(child);
      }

      call->~NodeFunctionCall();
      break;
    }
    case ASTNodeType::COMPOUND: {
      NodeCompound* compound = reinterpret_cast<NodeCompound*>(node.data);

      for (ASTNode child : compound->nodes) {
        destroyASTNode(child);
      }

      compound->~NodeCompound();
      break;
    }
    case ASTNodeType::UNKNOWN:
      break;
  }
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
      node.data =
          allocate(allocator, sizeof(NodeBinaryOp), alignof(NodeBinaryOp));
      new (node.data) NodeBinaryOp(*reinterpret_cast<NodeBinaryOp*>(data));
      break;
    case ASTNodeType::NUMBER:
      node.data = allocate(allocator, sizeof(NodeNumber), alignof(NodeNumber));
      new (node.data) NodeNumber(*reinterpret_cast<NodeNumber*>(data));
      break;
    case ASTNodeType::VAR: {
      node.data = allocate(allocator, sizeof(NodeVar), alignof(NodeVar));
      NodeVar* dataVar = reinterpret_cast<NodeVar*>(data);
      NodeVar* newVar = new (node.data) NodeVar();
      newVar->name = copyTokenString(dataVar->name);
      break;
    }
    case ASTNodeType::FUNCTION_DEFINITION:
      node.data = allocate(allocator, sizeof(NodeFunctionDefinition),
                           alignof(NodeFunctionDefinition));
      new (node.data) NodeFunctionDefinition(
          *reinterpret_cast<NodeFunctionDefinition*>(data));
      break;
    case ASTNodeType::COMPOUND:
      node.data =
          allocate(allocator, sizeof(NodeCompound), alignof(NodeCompound));
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

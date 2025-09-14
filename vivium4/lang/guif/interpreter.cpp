#include "interpreter.h"

namespace Vivium {
namespace GUIF {
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
    case ASTNodeType::ASSIGN_OP:
      return "";
    case ASTNodeType::VAR:
      return "";
    case ASTNodeType::NUMBER:
      // TODO: unsure on the usage here
      {
        NodeNumber* number = reinterpret_cast<NodeNumber*>(root.inplace);

        switch (number->value.type) {
          case TokenType::INTEGER:
            return std::format(
                "{}", *reinterpret_cast<uint64_t*>(number->value.inplace));
          case TokenType::FLOATING:
            return std::format(
                "{}", *reinterpret_cast<double*>(number->value.inplace));
          default:
            break;
        }
      }

      return "INVALID_NUMBER";
    case ASTNodeType::UNKNOWN:
      return "Unknown";
  }
}
}  // namespace GUIF
}  // namespace Vivium

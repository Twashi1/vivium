#pragma once

#include "ast.h"
#include "parser.h"

namespace Vivium {
namespace GUIF {
ASTNode evaluate(ASTNode root);
std::string printTree(ASTNode root);

}  // namespace GUIF
}  // namespace Vivium

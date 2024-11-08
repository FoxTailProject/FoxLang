#pragma once

#include <vector>

namespace FoxLang {
  enum class ASTNodeType;

  class ASTNode {
    ASTNodeType nodeType;
    std::vector<ASTNode> children;
    long int line;
  };

  enum class ASTNodeType {
    Function,
    Identifier,
    Assignment
  };
}

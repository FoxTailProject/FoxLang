#pragma once

#include <vector>

#include "../lexer/tokens.hpp"
#include "nodes.hpp"

namespace FoxLang {
  class ASTGenerator {
    public:
    ASTGenerator(std::vector<Token> *tokens);
    ASTFileNode GenerateFileTree();
    private:
    std::vector<Token> *tokens;
};
}

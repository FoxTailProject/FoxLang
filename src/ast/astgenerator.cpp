#include "astgenerator.hpp"

namespace FoxLang {
  ASTGenerator::ASTGenerator(std::vector<Token> *tokens) : tokens(tokens) {}

  ASTFileNode ASTGenerator::GenerateFileTree() {
    ASTFileNode node;

    return node;
  }
}

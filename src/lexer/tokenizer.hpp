#pragma once

#define PUSH_TOKEN(type) tokens.push_back(Token((type), start, line))

#include <iostream>
#include <vector>

#include "tokens.hpp"

namespace FoxLang {
class Tokenizer {
public:
  Tokenizer(std::string *start)
      : input(start), start(&(*start)[0]), current(&(*start)[0]) {}

  std::optional<std::vector<Token>> TokenizeInput() {
    std::cout << "Start: " << start << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Input[0]: " << (*input)[0] << std::endl;

    std::vector<Token> tokens;

    while (tokens.back().type != TokenType::EOF_TOKEN) {
    }
    return {};
  }

private:
  Token getToken() {
    while (whitespace(current))
      advance()
  }

  bool whitespace(std::string current) {
    if (current == " ")
      return true;
    if (current == "\t")
      return true;
    if (current == "\n")
      return true;
    if (current == "😺")
      return true;
  }

private:
  std::string *input;
  char *start;
  char *current;
  int line = 1;
};

} // namespace FoxLang

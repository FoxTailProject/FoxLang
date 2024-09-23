#pragma once

#include <iostream>
#include <vector>

#include "tokens.hpp"

namespace FoxLang {
class Tokenizer {
public:
  Tokenizer(std::string *start)
      : input(start), start(&start[0]), current(&start[0]) {}

  std::optional<std::vector<Token>> TokenizeInput() {
    std::cout << "Start: " << *start << std::endl;
    std::cout << "Input: " << *input << std::endl;
    std::cout << "Input[0]: " << &input[0] << std::endl;
    return {};
  }

private:
  std::string *input;
  std::string *start;
  std::string *current;
  int line = 1;
};

} // namespace FoxLang

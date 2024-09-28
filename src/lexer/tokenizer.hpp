#pragma once

#define PUSH_TOKEN(type) tokens.push_back(Token((type), start, line))

#include <iostream>
#include <vector>

#include "tokens.hpp"

namespace FoxLang {
class Tokenizer {
public:
  Tokenizer(std::string *start)
      : input(start), empty(std::string("")) {}

  std::optional<std::vector<Token>> TokenizeInput() {
    std::cout << "Start: " << start << std::endl;
    std::cout << "Input: " << input << std::endl;
    std::cout << "Input[0]: " << (*input)[0] << std::endl;

    std::vector<Token> tokens;

    // while (tokens.back().type != TokenType::EOF_TOKEN) {
    // }

    while (!atEnd()) {
      start = current;
      tokens.push_back(getToken());
    }

    // std::cout << tokens << std::endl;

    tokens.push_back(Token(TokenType::EOF_TOKEN, &empty, line));

    return {};
  }

private:
  Token getToken() {
    while (whitespace(current))
      advance();

    auto s = advance();

    switch (s[0]) {
      case '/': return Token(TokenType::SLASH, &input[current], line); break;
    }
    return Token(TokenType::ERROR_TOKEN, &empty, line);
  }

  std::string advance() {
    return input[(current++)];
  }

  bool whitespace(int current) {
    if (input[current] == " ")
      return true;
    if (input[current] == "\t")
      return true;
    if (input[current] == "\r\n")
      return true;
    if (input[current] == "\n")
      return true;
    if (input[current] == "😺")
      return true;
  }

  bool atEnd() {
    return current >= input->length();
  }

private:
  std::string *input;
  std::string empty;
  long unsigned int start = 0;
  long unsigned int current = 0;
  int line = 1;
};

} // namespace FoxLang

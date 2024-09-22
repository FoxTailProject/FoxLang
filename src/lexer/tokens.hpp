#pragma once

#include <optional>

namespace FoxLang {
// enum TokenType;
// class Token {
//   template <typename T> using Option = std::optional<T>;
//   Token(TokenType type, std::string lexeme, std::shared_ptr<void> literal)
// };

enum TokenType {
  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,

  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  AND,
  OR,

  // Bit manipulation
  LEFT_SHIFT,
  RIGHT_SHIFT,
  BIT_AND,
  BIT_OR,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  RETURN,
  SUPER,
  SELF,
  TRUE,
  VAR,
  WHILE,

  EOF
};
} // namespace FoxLang

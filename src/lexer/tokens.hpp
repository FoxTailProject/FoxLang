#pragma once

#include <optional>
#include <string>

namespace FoxLang {
enum class TokenType;

class Token {
public:
  Token(TokenType type, char *lexeme, int line)
      : type(type), lexeme(lexeme), line(line) {}

public:
  TokenType type;
  char *lexeme;
  int line;
  template <typename T> using Option = std::optional<T>;
};

enum class TokenType {
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
  COLON,
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

  EOF_TOKEN
};

} // namespace FoxLang

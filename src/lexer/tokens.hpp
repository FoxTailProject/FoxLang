#pragma once

#include <optional>
#include <string>

namespace FoxLang {
enum class TokenType;

class Token {
public:
	Token(TokenType type, std::string lexeme, int line)
		: type(type), lexeme(lexeme), line(line) {}

public:
	TokenType type;
	std::string lexeme;
	int line;
	template <typename T> using Option = std::optional<T>;
};

enum class TokenType {
	// Single-character tokens.
	LEFT_PAREN,
	RIGHT_PAREN,
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_SQUARE_BRACKET,
	RIGHT_SQUARE_BRACKET,
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
	BITWISE_AND,
	BITWISE_OR,

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

	EOF_TOKEN,
	ERROR_TOKEN,
};

} // namespace FoxLang

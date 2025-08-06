#pragma once

#include <optional>
#include <string>

namespace FoxLang {
enum class TokenType;

class Token {
public:
	Token(TokenType type, std::string lexeme, unsigned long line,
		  unsigned long column, unsigned long char_start, std::string fp)
		: type(type), lexeme(lexeme), line(line), column(column),
		  char_start(char_start), fp(fp) {}

public:
	TokenType type;
	std::string lexeme;
	unsigned long line;
	unsigned long column;
	unsigned long char_start;
	std::string fp;
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
	STRUCT,
	ELSE,
	FALSE,
	FUNC,
	EXTERN,
	FOR,
	IF,
	RETURN,
	SELF,
	TRUE,
	LET,
	CONST,
	MUT,
	WHILE,

	EOF_TOKEN,
	ERROR_TOKEN,
};

} // namespace FoxLang

#pragma once

#include <deque>
#include <fmt/format.h>
#include <string>
#include <vector>

#include "message.hpp"
#include "tokens.hpp"

namespace FoxLang {
class Lexer {
public:
	Lexer(std::string *source, std::string fp, std::deque<Message> &messages)
		: source(source), fp(fp), messages(messages) {}

	std::vector<Token> *Lex();

private:
	inline bool AtEnd();
	void currentToken();
	bool isDigit(char c);
	bool isAlpha(char c);
	bool isAlphaNumeric(char c);
	bool isAlphaNumericUnicode(char c);
	bool isANUcheckBack(int n);
	char peek();
	char peekNext(int n);
	char peekBack(int n);
	char advance();
	void addToken(TokenType token);
	bool match(char expected);
	char peekNext();
	void string();
	void number();
	void identifier();

private:
	std::string *source;
	std::string fp;
	std::vector<Token> tokens;
	std::deque<Message> messages;
	unsigned long int current = 0;
	unsigned long int start = 0;
	unsigned long line = 1;
	unsigned long column = 1;

	// clang-format off
	struct {
		std::string name;
		TokenType value;
	} keywords[14] = {
		{.name = "struct",	.value = TokenType::STRUCT},
		{.name = "else",	.value = TokenType::ELSE},
		{.name = "false",	.value = TokenType::FALSE},
		{.name = "for",		.value = TokenType::FOR},
		{.name = "fn",		.value = TokenType::FUNC},
		{.name = "if",		.value = TokenType::IF},
		{.name = "return",	.value = TokenType::RETURN},
		{.name = "self",	.value = TokenType::SELF},
		{.name = "true",	.value = TokenType::TRUE},
		{.name = "let",		.value = TokenType::LET},
		{.name = "const",	.value = TokenType::CONST},
		{.name = "while",	.value = TokenType::WHILE},
		{.name = "extern",	.value = TokenType::EXTERN},
		{.name = "mut",		.value = TokenType::MUT},
	};
	// clang-format on
};
} // namespace FoxLang

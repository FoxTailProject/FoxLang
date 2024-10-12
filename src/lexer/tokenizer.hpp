#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "tokens.hpp"

namespace FoxLang {
class Tokenizer {
public:
	Tokenizer(std::string *source) : source(source), tokens() {}

	std::vector<Token> *Tokenize() {
		while (!AtEnd()) {
			// std::cout << "1 "<< current << std::endl;
			start = current;
			currentToken();
			// std::cout << "2 "<< current << std::endl;
			// std::cout << current << ", " << source->length() <<
			// std::endl;
		}

		tokens.push_back(Token(TokenType::EOF_TOKEN, "", line));
		return &tokens;
	}

private:
	inline bool AtEnd() { return current >= source->length(); }

	void currentToken() {
		char c = advance();
		switch (c) {
		case '(':
			addToken(TokenType::LEFT_PAREN);
			break;
		case ')':
			addToken(TokenType::RIGHT_PAREN);
			break;
		case '{':
			addToken(TokenType::LEFT_BRACKET);
			break;
		case '}':
			addToken(TokenType::RIGHT_BRACKET);
			break;
		case '[':
			addToken(TokenType::LEFT_SQUARE_BRACKET);
			break;
		case ']':
			addToken(TokenType::RIGHT_SQUARE_BRACKET);
			break;
		case ',':
			addToken(TokenType::COMMA);
			break;
		case '.':
			addToken(TokenType::DOT);
			break;
		case '-':
			addToken(TokenType::MINUS);
			break;
		case '+':
			addToken(TokenType::PLUS);
			break;
		case ';':
			addToken(TokenType::SEMICOLON);
			break;
		case ':':
			addToken(TokenType::COLON);
			break;
		case '*':
			addToken(TokenType::STAR);
			break;
		case '&':
			addToken(match('&') ? TokenType::AND : TokenType::BITWISE_AND);
			break;
		case '|':
			addToken(match('|') ? TokenType::OR : TokenType::BITWISE_OR);
			break;
		case '!':
			addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
			break;
		case '=':
			addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
			break;
		case '<':
			addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
			break;
		case '>':
			addToken(match('=') ? TokenType::GREATER_EQUAL
								: TokenType::GREATER);
			break;
		case '/':
			if (match('/')) {
				// A comment goes until the end of the line.
				while (peek() != '\n' && !AtEnd())
					advance();
			} else if (match('*')) {
				while (peek() != '*' && peekNext(1) != '/' && !AtEnd())
					advance();
			} else {
				addToken(TokenType::SLASH);
			}
			break;
		case ' ':
		case '\r':
		case '\t':
			// Ignore whitespace.
			break;
		case "🦊"[0]: {
			// Cat emoji is 4 bytes long, need to check all four
			// chars
			if (match("🦊"[1]) && match("🦊"[2]) && match("🦊"[3]))
				break;
			// only unicode char that means anything, so it can be assumed that all other unicode chars are identifiers
			identifier();
		} break;
		case '\n':
			line++;
			break;
		case '"':
			string();
			break;
		default: {
			if (isDigit(c))
				number();
			else if (isAlpha(c))
				identifier();
			else
				std::cerr << "Unexpected character on line " << line << ": " << c
						  << std::endl;
		} break;
		}
	}

	bool isDigit(char c) { return c >= '0' && c <= '9'; }

	bool isAlpha(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
	}

	bool isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

	bool isAlphaNumericUnicode(char c) {
		if ((c & 0b10000000) == 0)
			return isAlpha(c) || isDigit(c);

		// first bytes to specify a multi-length char
		if ((c & 0b11000000) == 0b10000000) return true;
		if ((c & 0b11100000) == 0b11000000) return true;
		if ((c & 0b11110000) == 0b11100000) return true;
		if ((c & 0b11111000) == 0b11110000) return true;

		return isANUcheckBack(1);
	}

	bool isANUcheckBack(int n) {
		char c = peekBack(n);

		if ((c & 0b11000000) == 0b10000000) return true;
		if ((c & 0b11100000) == 0b11000000) return true;
		if ((c & 0b11110000) == 0b11100000) return true;
		if ((c & 0b11111000) == 0b11110000) return true;

		if ((c & 0b10000000) == 0) return isANUcheckBack(n + 1);
		return false;
	}

	char peek() {
		if (AtEnd()) return '\0';
		return source->at(current);
	}

	char peekNext(int n) {
		if (current + n >= source->length()) return '\0';
		return source->at(current + n);
	}

	char peekBack(int n) {
		return source->at(current - n);
	}

	char advance() {
		// if (current >= source->length()) return '\0';
		// std::cout << current << ", " << current + 1 << ", " <<
		// source->length()
		// << "\n" << std::endl; std::cout << source->at(current + 1) <<
		// std::endl;
		char c = source->at(current);
		current++;
		return c;
	}

	void addToken(TokenType token) {
		std::string substr = source->substr(start, current - start);
		tokens.push_back(Token(token, substr, line));
	}

	bool match(char expected) {
		if (AtEnd()) return false;
		if (source->at(current) != expected) return false;

		current++;
		return true;
	}

	char peekNext() {
		if (current + 1 >= source->length()) return '\0';
		return source->at(current + 1);
	}

	void string() {
		while (peek() != '"' && !AtEnd()) {
			if (peek() == '\n') line++;
			advance();
		}

		if (AtEnd()) {
			std::cerr << "Unterminated String on line " << line << std::endl;
			return;
		}

		// The closing ".
		advance();

		// Trim the surrounding quotes.
		// std::string value = source->substr(start + 1, current - 1);
		addToken(TokenType::STRING);
	}

	void number() {
		while (isDigit(peek()))
			advance();

		// Look for a fractional part.
		if (peek() == '.' && isDigit(peekNext())) {
			// Consume the "."
			advance();

			while (isDigit(peek()))
				advance();
		}

		addToken(TokenType::NUMBER);
	}

	void identifier() {
		while (isAlphaNumericUnicode(peek())) advance();

		std::string text = source->substr(start, current - start + 0);
		TokenType type = TokenType::IDENTIFIER;

		for (auto i : keywords) {
			if (i.name == text) {
				type = i.value;
				break;
			}
		}

		std::cout << (int)type << ", " << text << std::endl;

		addToken(type);
	}

private:
	std::string *source;
	std::vector<Token> tokens;
	unsigned long int current = 0;
	unsigned long int start = 0;
	int line = 1;

	struct { std::string name; TokenType value; } keywords[14] = {
		// { .name = "and", .value = TokenType::AND },
		{ .name = "class", .value = TokenType::CLASS },
		{ .name = "else", .value = TokenType::ELSE },
		{ .name = "false", .value = TokenType::FALSE },
		{ .name = "for", .value = TokenType::FOR },
		// { .name = "fun", .value = TokenType::FUN },
		{ .name = "if", .value = TokenType::IF },
		// { .name = "nil", .value = TokenType::NIL },
		// { .name = "or", .value = TokenType::OR },
		// { .name = "print", .value = TokenType::PRINT },
		{ .name = "return", .value = TokenType::RETURN },
		{ .name = "super", .value = TokenType::SUPER },
		{ .name = "self", .value = TokenType::SELF },
		{ .name = "true", .value = TokenType::TRUE },
		{ .name = "var", .value = TokenType::VAR },
		{ .name = "while", .value = TokenType::WHILE }
	};
};
} // namespace FoxLang

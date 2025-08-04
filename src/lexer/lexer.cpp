#include "lexer.hpp"

namespace FoxLang {
std::vector<Token> *Lexer::Lex() {
	while (!AtEnd()) {
		start = current;
		currentToken();
	}

	tokens.push_back(
		Token(TokenType::EOF_TOKEN, "", line, column, current, fp));
	return &tokens;
}

inline bool Lexer::AtEnd() { return current >= source->length(); }

void Lexer::currentToken() {
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
		addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
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
		if (match("🦊"[1]) && match("🦊"[2]) && match("🦊"[3])) break;
		// only unicode char that means anything, so it can be assumed that all
		// other unicode chars are identifiers
		identifier();
	} break;
	case '\n':
		line++;
		column = 1;
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
			messages.push_back(
				Message{.message = fmt::format("Unexpected character {}", c),
						.level = Severity::Error,
						.code = "E0001",
						.span = Location{
							.fp = fp,
							.line = line,
							.column = column,
							.char_start = current,
							.len = 1,
						}});
	} break;
	}
}

bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }

bool Lexer::isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }

bool Lexer::isAlphaNumericUnicode(char c) {
	if ((c & 0b10000000) == 0) return isAlpha(c) || isDigit(c);

	// first bytes to specify a multi-length char
	if ((c & 0b11000000) == 0b10000000) return true;
	if ((c & 0b11100000) == 0b11000000) return true;
	if ((c & 0b11110000) == 0b11100000) return true;
	if ((c & 0b11111000) == 0b11110000) return true;

	return isANUcheckBack(1);
}

bool Lexer::isANUcheckBack(int n) {
	char c = peekBack(n);

	if ((c & 0b11000000) == 0b10000000) return true;
	if ((c & 0b11100000) == 0b11000000) return true;
	if ((c & 0b11110000) == 0b11100000) return true;
	if ((c & 0b11111000) == 0b11110000) return true;

	if ((c & 0b10000000) == 0) return isANUcheckBack(n + 1);
	return false;
}

char Lexer::peek() {
	if (AtEnd()) return '\0';
	return source->at(current);
}

char Lexer::peekNext(int n) {
	if (current + n >= source->length()) return '\0';
	return source->at(current + n);
}

char Lexer::peekBack(int n) { return source->at(current - n); }

char Lexer::advance() {
	char c = source->at(current);
	current++;
	column++;
	return c;
}

void Lexer::addToken(TokenType token) {
	std::string substr = source->substr(start, current - start);
	tokens.push_back(Token(token, substr, line, column, current, fp));
}

bool Lexer::match(char expected) {
	if (AtEnd()) return false;
	if (source->at(current) != expected) return false;

	current++;
	return true;
}

char Lexer::peekNext() {
	if (current + 1 >= source->length()) return '\0';
	return source->at(current + 1);
}

void Lexer::string() {
	while (peek() != '"' && !AtEnd()) {
		if (peek() == '\n') {
			line++;
			column = 1;
		}
		advance();
	}

	if (AtEnd()) {
		messages.push_back(
			Message{.message = fmt::format("Unterminated string"),
					.level = Severity::Error,
					.code = "E0002",
					.span = Location{
						.fp = fp,
						.line = line,
						.column = column,
						.char_start = current,
						.len = 1,
					}});
		return;
	}

	// The closing ".
	advance();

	// Trim the surrounding quotes.
	// std::string value = source->substr(start + 1, current - 1);
	addToken(TokenType::STRING);
}

void Lexer::number() {
	while (isDigit(peek()))
		advance();

	// Look for a fractional part.
	if (peek() == '.' && Lexer::isDigit(peekNext())) {
		// Consume the "."
		advance();

		while (isDigit(peek()))
			advance();
	}

	addToken(TokenType::NUMBER);
}

void Lexer::identifier() {
	while (isAlphaNumericUnicode(peek()))
		advance();

	std::string text = source->substr(start, current - start + 0);
	TokenType type = TokenType::IDENTIFIER;

	for (auto i : keywords) {
		if (i.name == text) {
			type = i.value;
			break;
		}
	}

	// std::cout << (int)type << ", " << text << std::endl;

	addToken(type);
}
} // namespace FoxLang

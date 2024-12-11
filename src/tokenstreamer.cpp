#include "tokenstreamer.hpp"

#include <iostream>

namespace FoxLang {
TokenStreamer::TokenStreamer(std::vector<Token> t) : tokens(t), current(0) {}

Token *TokenStreamer::peek() {
	std::cout << (int)tokens[0].type << std::endl;
	std::cout << tokens.size() << std::endl;
	std::cout << (int)tokens.back().type << std::endl;
	if (current == tokens.size() - 1) return &tokens.back();
	return &tokens[current + 1];
}

Token *TokenStreamer::advance() {
	current++;
	return &tokens[current];
}

Token *TokenStreamer::currentToken() { return &tokens[current]; }

bool TokenStreamer::match(TokenType type) {
	if (peek()->type == type) {
		advance();
		return true;
	}
	return false;
}
} // namespace FoxLang

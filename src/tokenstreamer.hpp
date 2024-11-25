#pragma once

#include "lexer/tokens.hpp"
#include <vector>

namespace FoxLang {
class TokenStreamer {
public:
	TokenStreamer(std::vector<Token>);
  Token peek();

private:
	std::vector<Token> tokens;
	long long unsigned int current;
};
} // namespace FoxLang

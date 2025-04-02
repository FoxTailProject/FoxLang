#pragma once

#include <stack>
#include <vector>

namespace FoxLang::IR {
class Builder {
private:
	std::stack<Namespace *> nsStack;
	Namespace *currentNamespace;
	std::vector<Namespace> nses;
	std::vector<Instruction> intstructions;
};
} // namespace FoxLang::IR

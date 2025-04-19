#pragma once

#include <stack>
#include <string>
#include <vector>

#include "namespace.h"

namespace FoxLang::IR {
class Builder {
public:
	Builder()
		: nsStack(), currentNamespace(nullptr), insertPoint(nullptr), nses() {}

	void createFunction(std::string name);

private:
	std::stack<Namespace *> nsStack;
	Namespace *currentNamespace;
	void *insertPoint;
	std::vector<Namespace> nses;
};
} // namespace FoxLang::IR

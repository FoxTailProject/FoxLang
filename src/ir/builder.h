#pragma once

#include <stack>
#include <string>
#include <vector>

#include "container.h"
#include "namespace.h"
#include "value.h"

namespace FoxLang::IR {
class Builder {
public:
	Builder()
		: nsStack(), currentNamespace(nullptr), ipStack(), insertPoint(nullptr),
		  nses() {}

	Container *createFunction(std::string name);
	void createAdd(Value *left, Value *right);
	void createSub(Value *left, Value *right);
	void createMul(Value *left, Value *right);
	void createDiv(Value *left, Value *right);
	Container *createStruct(std::string name);
	void createFunctionCall(Container *function);
	Value *createVariable(std::string name);

private:
	std::stack<Namespace *> nsStack;
	std::stack<Container *> ipStack;
	Namespace *currentNamespace;
	Container *insertPoint;
	std::vector<Namespace> nses;
};
} // namespace FoxLang::IR

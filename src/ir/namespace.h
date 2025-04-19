#pragma once

#include <string>
#include <vector>

#include "container.h"

namespace FoxLang::IR {
class Namespace {
public:
	void createFunction(std::string name);

private:
	std::vector<Container *> containers;
};
} // namespace FoxLang::IR

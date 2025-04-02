#pragma once

#include <vector>

#include "container.h"

namespace FoxLang::IR {
class Namespace {
private:
	std::vector<Container *> containers;
};
} // namespace FoxLang::IR

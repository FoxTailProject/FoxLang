#include "builder.h"

namespace FoxLang::IR {
void Builder::createFunction(std::string name) {
	currentNamespace->createFunction(name);
	return;
}
} // namespace FoxLang::IR

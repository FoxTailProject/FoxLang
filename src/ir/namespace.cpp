#include "namespace.h"

namespace FoxLang::IR {
void Namespace::createFunction(std::string name) {
	Container *c = new Container();
	c->type = ContainerType::Function;
	containers.push_back(c);
}
} // namespace FoxLang::IR

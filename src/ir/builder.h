#pragma once

namespace FoxLangIR {
class Builder {
private:
	Namespace currentNamespace;
	std::vector<Instruction> intstructions;
}
} // namespace FoxLangIR

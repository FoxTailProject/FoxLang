#pragma once

namespace FoxLang::IR {
enum class ContainerType;

class Container {
public:
	~Container() = default;
	ContainerType type;
	void *stuff;
};

enum class ContainerType { Function, Struct };
} // namespace FoxLang::IR

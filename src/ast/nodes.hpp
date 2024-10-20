#pragma once

#include <vector>

namespace FoxLang {
enum class NodeType;

class ASTFileNode {};

class ASTNode {
private:
	NodeType type;
	std::vector<ASTNode> children;
};

enum class NodeType { FuncDec };
} // namespace FoxLang

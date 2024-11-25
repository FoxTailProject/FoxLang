#pragma once
#include <vector>

#include "../tokenstreamer.hpp"

namespace FoxLang {
enum class ASTNodeType;

class ASTNode {
public:
	ASTNode(ASTNodeType, TokenStreamer *, long unsigned int);

private:
	ASTNodeType nodeType;
	TokenStreamer *streamer;
	std::vector<ASTNode> children;
	long unsigned int line;
	std::vector<void *> data;
};

enum class ASTNodeType {
	Function,
	Identifier,
	Assignment,
	Block,
	File,
	Number,
	Variable,
	BinaryExpr,
	CallExpr,
	Prototype
};
} // namespace FoxLang

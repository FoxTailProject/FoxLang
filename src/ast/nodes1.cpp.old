#include "nodes.hpp"

namespace FoxLang {
namespace ASTNodes {
ASTNode File(TokenStreamer *);
ASTNode BinExpr(TokenStreamer *);
} // namespace ASTNodes

ASTNode::ASTNode(ASTNodeType type, TokenStreamer *streamer,
				 long unsigned int line)
	: nodeType(type), streamer(streamer), line(line) {
	switch (type) {
	case ASTNodeType::File:
		ASTNodes::File(streamer);
		break;
	case ASTNodeType::BinaryExpr:
		data.push_back((void *)&op);
		ASTNodes::BinExpr(streamer);
		break;
	};
}

ASTNode ASTNodes::File(TokenStreamer *streamer) {
	auto next = streamer->peek();
	switch (next.type) {
	case TokenType::FUNC: {
		// :)
	} break;
	};
}

ASTNode BinExpr(TokenStreamer *tokens, char op) { data.push((void *)&op); }
} // namespace FoxLang

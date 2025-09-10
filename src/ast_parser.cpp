#include "ast_parser.hpp"
#include "ast_nodes.hpp"
#include <fmt/format.h>
#include <iostream>

namespace FoxLang {
std::optional<std::shared_ptr<ExprAST>> Parser::parseNumberExpr() {
	auto ret = std::make_shared<NumberExprAST>(current->lexeme);
	current++;
	return ret;
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseParenExpr() {
	current++;
	auto expression = parseExpression();

	if (!expression) return std::nullopt;

	if (current->type != TokenType::RIGHT_PAREN) {
		LogError("Expected closing parenthesis", "E0003");
		return std::nullopt;
	}

	current++;
	return expression;
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseIdentifierExpr() {
	std::string identifierString = current->lexeme;
	current++;

	if (current->type != TokenType::LEFT_PAREN) // Simple variable ref.
		return std::make_shared<VariableExprAST>(identifierString);
	current++;

	// Call.
	std::vector<std::shared_ptr<ExprAST>> Args;
	// if (current->type != TokenType::RIGHT_PAREN) {
	// while (true) {
	// if (auto Arg = parseExpression().value())
	// Args.push_back(std::move(Arg));
	// else
	// return std::nullopt;
	//
	// if (current->type == TokenType::RIGHT_PAREN) break;
	//
	// if (current->type != TokenType::COMMA) {
	// LogError("Expected ')' or ',' in argument list");
	// return std::nullopt;
	// }
	// current++;
	// }
	// }

	while (current->type != TokenType::RIGHT_PAREN) {
		auto arg = parseExpression();
		if (!arg) {
			LogError("unable to parse expression", "E0100");
			return std::nullopt;
		}
		Args.push_back(std::move(arg.value()));

		if (current->type == TokenType::COMMA) current++;
	}

	// Eat the ')'.
	current++;

	return std::make_shared<CallExprAST>(identifierString, std::move(Args));
}

std::optional<std::shared_ptr<ExprAST>> Parser::parsePrimary() {
	switch (current->type) {
	default:
		LogError(
			fmt::format("unknown token {} when trying to parse an expression",
						current->lexeme),
			"E0102");
		return std::nullopt;
	case TokenType::IDENTIFIER:
		return parseIdentifierExpr();
	case TokenType::NUMBER:
		return parseNumberExpr();
	case TokenType::LEFT_PAREN:
		return parseParenExpr();
	case TokenType::STRING: {
		auto c = current->lexeme;
		current++;
		return std::make_shared<StringLiteralAST>(c);
	}
	case TokenType::TRUE: {
		current++;
		return std::make_shared<BoolLiteralAST>(true);
	}
	case TokenType::FALSE: {
		current++;
		return std::make_shared<BoolLiteralAST>(false);
	}
	case TokenType::DOT: {
		if ((current + 1)->type == TokenType::LEFT_BRACKET) {
			return parseStructInstance();
		}
	} break;
	}
}

std::optional<std::shared_ptr<StructLiteralAST>> Parser::parseStructInstance() {
	if (current->type != TokenType::DOT ||
		(current + 1)->type != TokenType::LEFT_BRACKET) {
		// TODO: maybe log an error? i mean it shouldnt get called
		return std::nullopt;
	}

	// .{
	current += 2;

	std::vector<std::string> names;
	std::vector<std::shared_ptr<ExprAST>> vals;

	while (current->type != TokenType::RIGHT_BRACKET) {
		if (current->type != TokenType::DOT)
			LogError("Dot before struct member name required", "E0015");
		else
			current++;

		names.push_back(current->lexeme);
		current++;

		if (current->type != TokenType::EQUAL)
			LogError("Expected = after name", "E0016");
		else
			current++;

		auto val = parseExpression();

		if (!val) {
			current++;
			continue;
		}

		if (current->type != TokenType::COMMA)
			LogError("Comma after each struct member required", "E0016");
		else
			current++;

		vals.push_back(val.value());
	}

	current++;

	return std::make_shared<StructLiteralAST>(names, vals);
}

std::optional<std::shared_ptr<ExprAST>> Parser::parseExpression() {
	auto lhs = parsePrimary();
	if (!lhs) return std::nullopt;

	return parseBinOpRHS(0, std::move(lhs));
}

std::optional<std::shared_ptr<StmtAST>> Parser::parseStatement() {
	switch (current->type) {
	case TokenType::LET:
		return parseLet();
	case TokenType::RETURN:
		return parseReturnStmt();
	case TokenType::IF:
		return parseIfStmt();
	case TokenType::WHILE:
		return parseWhileStmt();
	default:
		return parseExprStatement();
	}
}

std::optional<std::shared_ptr<ExprStmt>> Parser::parseExprStatement() {
	auto expr = parseExpression();
	if (!expr) {
		LogError("Expected expression", "E0103");
		return std::nullopt;
	}

	if (current->type != TokenType::SEMICOLON) {
		LogError("expected ; after expression", "E0005");
		return std::nullopt;
	}
	current++;

	return std::make_shared<ExprStmt>(std::move(expr.value()));
}

std::optional<std::shared_ptr<BlockAST>> Parser::parseBlock() {
	if (current->type != TokenType::LEFT_BRACKET) {
		LogError("Expected '{' to start block", "E0006");
		return std::nullopt;
	}
	current++;

	std::vector<std::shared_ptr<StmtAST>> content;

	while (current->type != TokenType::RIGHT_BRACKET) {
		auto stmt = parseStatement();
		// if (!stmt) {
		// LogError("Expected expression in block", "E0104");
		// return std::nullopt;
		// }

		if (stmt) content.push_back(std::move(stmt.value()));
	}

	current++;
	return std::make_shared<BlockAST>(std::move(content), false);
}

std::optional<std::shared_ptr<BlockAST>> Parser::parseBklessBlock() {
	if (current->type == TokenType::LEFT_BRACKET) return parseBlock();

	auto stmt = parseStatement();
	if (!stmt) {
		LogError("Unable to parse bracketless block, unable to parse statement",
				 "E0105");
		return std::nullopt;
	}

	return std::make_shared<BlockAST>(
		std::vector<std::shared_ptr<StmtAST>>{stmt.value()}, true);
}

std::optional<std::shared_ptr<ExprAST>>
Parser::parseBinOpRHS(int precedence,
					  std::optional<std::shared_ptr<ExprAST>> lhs) {
	while (true) {
		int currentPrecedence = getPrecedence(current->type);

		if (currentPrecedence < precedence) return lhs;

		Token binaryOperator = *current;
		current++;

		auto rhs = parsePrimary();
		if (!rhs) return std::nullopt;

		if (precedence < getPrecedence(current->type)) {
			rhs = parseBinOpRHS(precedence + 1, std::move(rhs));
			if (!rhs) return std::nullopt;
		}

		lhs = std::make_shared<BinaryExprAST>(
			binaryOperator, std::move(lhs.value()), std::move(rhs.value()));
	}
}

std::optional<std::shared_ptr<TypeAST>> Parser::parseType() {
	std::cout << "Type " << current->lexeme << std::endl;
	if (current->type == TokenType::LEFT_SQUARE_BRACKET) {
		auto count = parseNumberExpr();
		current++;
		auto child = parseType();
		return std::nullopt;
	}

	bool pointer = false;
	if (current->type == TokenType::BITWISE_AND) {
		pointer = true;
		current++;
	}

	if (current->type != TokenType::IDENTIFIER) {
		LogError("Unable to parse type", "E0200");
		return std::nullopt;
	}

	std::string type = current->lexeme;
	TypeAST::Type t = TypeAST::Type::_struct;
	current++;

	for (auto i : TypeAST::conversion) {
		if (i.name == type) {
			t = i.value;
			break;
		}
	}

	if (pointer)
		return std::make_shared<TypeAST>(
			TypeAST::Type::pointer,
			std::make_shared<TypeAST>(t, std::nullopt, type), "&");

	return std::make_shared<TypeAST>(t, std::nullopt, type);
}

std::optional<std::shared_ptr<StructAST>> Parser::parseStruct() {
	current++;

	std::string name = current->lexeme;
	if (current->type != TokenType::IDENTIFIER) {
		LogError("Expected name for struct", "E0112");
	}

	current++;
	if (current->type != TokenType::LEFT_BRACKET) {
		LogError("Expected bracket in struct definition", "E0113");
	}

	current++;
	std::vector<std::shared_ptr<StructMemberAST>> members;

	while (current->type != TokenType::RIGHT_BRACKET) {
		auto name = current->lexeme;
		if (current->type != TokenType::IDENTIFIER)
			LogError("Expected name for element in struct", "E0400");
		current++;

		auto type = parseType();
		if (!type) {
			current++;
			continue;
		}

		if (current->type != TokenType::COMMA)
			LogError("Expected a comma after every element in struct", "E0010");
		else
			current++;

		members.push_back(
			std::make_shared<StructMemberAST>(name, type.value()));
	}

	current++;

	return std::make_shared<StructAST>(name, members);
}

std::optional<std::shared_ptr<VarDecl>> Parser::parseLet() {
	// Move past `let` token, onto either a `mut` token or the var name
	current++;

	bool mut = false;
	if (current->type == TokenType::MUT) {
		mut = true;
		current++;
	}

	std::string name = current->lexeme;
	current++;
	std::optional<std::shared_ptr<TypeAST>> type = parseType();

	if (!type) {
		LogError("Type inference is not yet implemented. If you wrote Haskell, "
				 "send me an email",
				 "ESMUT");
		return std::nullopt;
	}

	if (current->type == TokenType::SEMICOLON) {
		return std::make_shared<VarDecl>(name, type.value(), std::nullopt, mut);
	}

	if (current->type != TokenType::EQUAL) {
		LogError("You need an equal sign zidiot", "E0006");
		return std::nullopt;
	}
	current++; // move past = onto the expression

	auto value = parseExpression();
	if (!value) {
		LogError("You need an expression after the =", "E0007");
		LogError("Im done with errors rn", "E0007");
		return std::nullopt;
	}
	current++;

	return std::make_shared<VarDecl>(name, type.value(), std::move(value), mut);
}

std::optional<std::shared_ptr<IfStmt>> Parser::parseIfStmt() {
	current++; // move past if statement

	auto cond = parseExpression();
	if (!cond) {
		LogError("Need a condition for an if statement", "E0106");
	}

	auto block = parseBklessBlock();
	if (!block) {
		LogError("Need a block for an if statement", "E0107");
	}

	std::optional<std::shared_ptr<BlockAST>> else_ = std::nullopt;
	if (current->type == TokenType::ELSE) {
		current++;					// Move past else
		else_ = parseBklessBlock(); // If else chains will have the if stored
									// inside the else of the previous if
		if (!else_) {
			LogError("Unable to parse block inside else", "E0108");
		}
	}

	if (!cond || !block) return std::nullopt;
	return std::make_shared<IfStmt>(std::move(cond.value()),
									std::move(block.value()), std::move(else_));
}

std::optional<std::shared_ptr<WhileStmt>> Parser::parseWhileStmt() {
	current++; // move past while statement

	auto cond = parseExpression();
	if (!cond) {
		LogError("Need a condition for a while statment", "E0112");
	}

	auto block = parseBklessBlock();
	if (!block) {
		LogError("Need a block for a while statement", "E0113");
	}

	if (!cond || !block) return std::nullopt;
	return std::make_shared<WhileStmt>(std::move(cond.value()),
									   std::move(block.value()));
}

std::optional<std::shared_ptr<PrototypeAST>> Parser::parsePrototype() {
	if (current->type != TokenType::IDENTIFIER) {
		LogError("Expected function name in prototype", "E0109");
		return std::nullopt;
	}

	std::string name = current->lexeme;
	current++;

	if (current->type != TokenType::LEFT_PAREN) {
		LogError("Expected '(' in function prototype", "E0008");
		return std::nullopt;
	}

	std::vector<std::string> argNames;
	std::vector<std::shared_ptr<TypeAST>> typeNames;
	current++; // Consume the ( and begin parsing the args

	while (current->type == TokenType::IDENTIFIER) {
		argNames.push_back(current->lexeme);
		current++; // Consume the IDENTIFIER arg name

		auto type = parseType(); // Consumes IDENTIFIERs for the type
		if (!type) {
			LogError("Unable to parse type", "E0110");
			return std::nullopt;
		}

		typeNames.push_back(std::move(type.value()));

		if (current->type != TokenType::COMMA) {
			break;
		}

		current++;
	}

	if (current->type != TokenType::RIGHT_PAREN) {
		LogError("Expected ')' in function prototype", "E0009");
		return std::nullopt;
	}

	current++;
	auto retType = parseType();
	std::cout << "Ret type " << retType.value()->data << " for func " << name
			  << std::endl;
	if (!retType) {
		LogError("Unable to parse return type of function", "E0111");
		return std::nullopt;
	}

	std::vector<std::shared_ptr<ParameterAST>> params;
	for (int i = 0; i < argNames.size(); ++i) {
		params.push_back(
			std::make_shared<ParameterAST>(argNames[i], typeNames[i]));
	}

	return std::make_shared<PrototypeAST>(name, std::move(params),
										  std::move(retType.value()));
}

std::optional<std::shared_ptr<FunctionAST>> Parser::parseDefinition() {
	current++;
	auto proto = parsePrototype();
	if (!proto) return std::nullopt;

	auto expr = parseBlock();
	if (!expr) return std::nullopt;

	return std::make_shared<FunctionAST>(std::move(proto.value()),
										 std::move(expr.value()));
}

std::optional<std::shared_ptr<ReturnStmt>> Parser::parseReturnStmt() {
	current++;

	if (current->type == TokenType::SEMICOLON) {
		current++;
		return std::make_shared<ReturnStmt>(std::nullopt);
	}

	auto expr = parseExpression();
	// if (!expr) return std::nullopt;
	current++;
	return std::make_shared<ReturnStmt>(std::move(expr));
}

FileAST *Parser::parse() {
	std::vector<std::shared_ptr<AST>> fileNodes;
	while (true) {
		switch (current->type) {
		case TokenType::EOF_TOKEN:
			return new FileAST("test", std::move(fileNodes));
		case TokenType::SEMICOLON:
			current++;
			break;
		case TokenType::FUNC: {
			auto definition = parseDefinition();
			if (!definition) continue;
			fileNodes.push_back(std::move(definition.value()));
			// fileNodes.push_back(
			// static_cast<ExprAST *>(definition.value().release()));
		} break;
		case TokenType::STRUCT: {
			auto def = parseStruct();
			fileNodes.push_back(std::move(def.value()));
		} break;
		default: {
			LogError(fmt::format("Unexpected character '{}'", current->lexeme),
					 "E0010");
			current++;
		} break;
		}
	}
}

void Parser::LogError(std::string message, std::string code) {
	std::cout << "Error " << code << ": " << message << std::endl;
	messages.push_back(
		Message{.message = message,
				.level = Severity::Error,
				.code = code,
				.span = Location{
					.fp = current->fp,
					.line = current->line,
					.column = current->column - current->lexeme.length(),
					.start = current->char_start - current->lexeme.length(),
					.end = current->char_start,
				}});
}

void Parser::LogWarning(std::string message, std::string code) {
	messages.push_back(
		Message{.message = message,
				.level = Severity::Warning,
				.code = code,
				.span = Location{
					.fp = current->fp,
					.line = current->line,
					.column = current->column - current->lexeme.length(),
					.start = current->char_start - current->lexeme.length(),
					.end = current->char_start,
				}});
}
} // namespace FoxLang

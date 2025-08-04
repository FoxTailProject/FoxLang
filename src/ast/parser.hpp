#pragma once

#include "../message.hpp"
#include "nodes.hpp"

#include <deque>
#include <memory>
#include <queue>

namespace FoxLang {
class Parser {
public:
	Parser(std::vector<Token> *tokens, std::deque<Message> &messages)
		: current(tokens->begin()), messages(messages) {}

	FileAST *parse();

private:
	std::vector<Token>::iterator current;
	std::deque<Message> &messages;

private:
	std::optional<std::shared_ptr<ExprAST>> parseNumberExpr();
	std::optional<std::shared_ptr<ExprAST>> parseParenExpr();
	std::optional<std::shared_ptr<ExprAST>> parseIdentifierExpr();
	std::optional<std::shared_ptr<ExprAST>> parsePrimary();
	std::optional<std::shared_ptr<ExprAST>> parseExpression();
	std::optional<std::shared_ptr<StmtAST>> parseStatement();
	std::optional<std::shared_ptr<ExprStmt>> parseExprStatement();
	std::optional<std::shared_ptr<BlockAST>> parseBlock();
	std::optional<std::shared_ptr<BlockAST>> parseBklessBlock();
	std::optional<std::shared_ptr<VarDecl>> parseLet();
	std::optional<std::shared_ptr<IfStmt>> parseIfStmt();
	std::optional<std::shared_ptr<TypeAST>> parseType();
	std::optional<std::shared_ptr<PrototypeAST>> parsePrototype();
	std::optional<std::shared_ptr<FunctionAST>> parseDefinition();
	std::optional<std::shared_ptr<ReturnStmt>> parseReturnStmt();

	std::optional<std::shared_ptr<ExprAST>>
	parseBinOpRHS(int, std::optional<std::shared_ptr<ExprAST>>);

	void LogError(std::string message, std::string code);
	void LogWarning(std::string message, std::string code);
};
} // namespace FoxLang

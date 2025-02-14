#pragma once

#include "nodes.h"
#include <memory>

namespace FoxLang {
class Parser {
public:
	Parser(std::vector<Token> *);
	FileAST *parse();

private:
	std::vector<Token> *tokens;
	std::vector<Token>::iterator current;

private:
	std::optional<std::shared_ptr<ExprAST>> parseNumberExpr();
	std::optional<std::shared_ptr<ExprAST>> parseParenExpr();
	std::optional<std::shared_ptr<ExprAST>> parseIdentifierExpr();
	std::optional<std::shared_ptr<ExprAST>> parsePrimary();
	std::optional<std::shared_ptr<ExprAST>> parseExpression();
	std::optional<std::shared_ptr<StmtAST>> parseStatement();
	std::optional<std::shared_ptr<ExprStmt>> parseExprStatement();
	std::optional<std::shared_ptr<BlockAST>> parseBlock();
	std::optional<std::shared_ptr<VarDecl>> parseLet();
	std::optional<std::shared_ptr<TypeAST>> parseType();
	std::optional<std::shared_ptr<PrototypeAST>> parsePrototype();
	std::optional<std::shared_ptr<FunctionAST>> parseDefinition();
	std::optional<std::shared_ptr<ReturnStmt>> parseReturnStmt();

	std::optional<std::shared_ptr<ExprAST>>
	parseBinOpRHS(int, std::optional<std::shared_ptr<ExprAST>>);
};
} // namespace FoxLang

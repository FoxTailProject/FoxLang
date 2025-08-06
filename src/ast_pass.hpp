#pragma once

#include "ast_nodes.hpp"

namespace FoxLang {
class ASTVisitor {
public:
	virtual void visit(BlockAST &it) = 0;
	virtual void visit(BinaryExprAST &it) = 0;
	virtual void visit(CallExprAST &it) = 0;
	virtual void visit(NumberExprAST &it) = 0;
	virtual void visit(VariableExprAST &it) = 0;
	virtual void visit(FileAST &it) = 0;
	virtual void visit(FunctionAST &it) = 0;
	virtual void visit(PrototypeAST &it) = 0;
	virtual void visit(ExprStmt &it) = 0;
	virtual void visit(Literal &it) = 0;
	virtual void visit(ReturnStmt &it) = 0;
	virtual void visit(IfStmt &it) = 0;
	virtual void visit(WhileStmt &it) = 0;
	virtual void visit(VarDecl &it) = 0;
	virtual void visit(TypeAST &it) = 0;
};
} // namespace FoxLang

#pragma once

#include "ast_nodes.hpp"
#include "ast_pass.hpp"
#include "message.hpp"
#include <stack>

namespace FoxLang {
class NameResolution : public ASTVisitor {
	typedef std::map<std::string, AST *> Scope;

	std::deque<Scope> scopes;
	Scope global_scope;
	Scope function_scope;
	std::deque<Message> &messages;

public:
	NameResolution(std::deque<Message> &m) : messages(m) {}

	virtual void visit(BlockAST &it);
	virtual void visit(BinaryExprAST &it);
	virtual void visit(CallExprAST &it);
	virtual void visit(NumberExprAST &it);
	virtual void visit(StringLiteralAST &it);
	virtual void visit(BoolLiteralAST &it);
	virtual void visit(StructLiteralAST &it);
	virtual void visit(VariableExprAST &it);
	virtual void visit(FileAST &it);
	virtual void visit(ParameterAST &it);
	virtual void visit(FunctionAST &it);
	virtual void visit(PrototypeAST &it);
	virtual void visit(ExprStmt &it);
	virtual void visit(ReturnStmt &it);
	virtual void visit(IfStmt &it);
	virtual void visit(WhileStmt &it);
	virtual void visit(VarDecl &it);
	virtual void visit(TypeAST &it);
	virtual void visit(StructMemberAST &it);
	virtual void visit(StructAST &it);

	void depth_proto(PrototypeAST &);
	void depth_struct(StructAST &);
};
} // namespace FoxLang

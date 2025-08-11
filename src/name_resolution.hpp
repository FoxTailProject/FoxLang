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

	void visit(BlockAST &it) override;
	void visit(BinaryExprAST &it) override;
	void visit(CallExprAST &it) override;
	void visit(NumberExprAST &it) override;
	void visit(VariableExprAST &it) override;
	void visit(FileAST &it) override;
	void visit(ParameterAST &it) override;
	void visit(FunctionAST &it) override;
	void visit(PrototypeAST &it) override;
	void visit(ExprStmt &it) override;
	void visit(Literal &it) override;
	void visit(ReturnStmt &it) override;
	void visit(IfStmt &it) override;
	void visit(WhileStmt &it) override;
	void visit(VarDecl &it) override;
	void visit(TypeAST &it) override;
	void visit(StructAST &it) override;

	void depth_proto(PrototypeAST &);
	void depth_struct(StructAST &);
};
} // namespace FoxLang

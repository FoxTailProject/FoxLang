#pragma once

#include "ast/nodes.hpp"

namespace FoxLang {
class ASTVisitor {
public:
	virtual llvm::Value *visit(BlockAST &it) = 0;
	virtual llvm::Value *visit(BinaryExprAST &it) = 0;
	virtual llvm::Value *visit(CallExprAST &it) = 0;
	virtual llvm::Value *visit(NumberExprAST &it) = 0;
	virtual llvm::Value *visit(VariableExprAST &it) = 0;
	virtual llvm::Value *visit(FileAST &it) = 0;
	virtual llvm::Value *visit(FunctionAST &it) = 0;
	virtual llvm::Value *visit(PrototypeAST &it) = 0;
	virtual llvm::Value *visit(ExprStmt &it) = 0;
	virtual llvm::Value *visit(ReturnStmt &it) = 0;
	virtual llvm::Value *visit(IfStmt &it) = 0;
	virtual llvm::Value *visit(VarDecl &it) = 0;
	virtual llvm::Value *visit(TypeAST &it) = 0;
};
} // namespace FoxLang

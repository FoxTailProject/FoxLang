#pragma once

#include "ast_nodes.hpp"
#include "ast_pass.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

namespace FoxLang::IR {
class Generator : public ASTVisitor {
public:
	static std::unique_ptr<llvm::LLVMContext> context;
	static std::unique_ptr<llvm::IRBuilder<>> builder;
	static std::unique_ptr<llvm::Module> llvm_module;
	llvm::Value *returned;
	llvm::Type *returned_type;

public:
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
	virtual void visit(StructAST &it);
	virtual void visit(StructMemberAST &it);
};
} // namespace FoxLang::IR

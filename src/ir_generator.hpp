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

public:
	virtual void visit(BlockAST &it) override;
	virtual void visit(BinaryExprAST &it) override;
	virtual void visit(CallExprAST &it) override;
	virtual void visit(NumberExprAST &it) override;
	virtual void visit(VariableExprAST &it) override;
	virtual void visit(FileAST &it) override;
	virtual void visit(ParameterAST &it) override;
	virtual void visit(FunctionAST &it) override;
	virtual void visit(PrototypeAST &it) override;
	virtual void visit(ExprStmt &it) override;
	virtual void visit(Literal &it) override;
	virtual void visit(ReturnStmt &it) override;
	virtual void visit(IfStmt &it) override;
	virtual void visit(WhileStmt &it) override;
	virtual void visit(VarDecl &it) override;
	virtual void visit(TypeAST &it) override;
	virtual void visit(StructAST &it) override;
};
} // namespace FoxLang::IR

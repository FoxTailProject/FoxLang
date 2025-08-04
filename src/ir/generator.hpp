#pragma once

#include "../ast/nodes.hpp"
#include "../ast_pass.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>

namespace FoxLang::IR {
class Generator : public ASTVisitor {
	static std::unique_ptr<llvm::LLVMContext> context;
	static std::unique_ptr<llvm::IRBuilder<>> builder;
	static std::unique_ptr<llvm::Module> llvm_module;
	static std::map<std::string, llvm::Value *> named_values;

public:
	virtual llvm::Value *visit(BlockAST &it) override;
	virtual llvm::Value *visit(BinaryExprAST &it) override;
	virtual llvm::Value *visit(CallExprAST &it) override;
	virtual llvm::Value *visit(NumberExprAST &it) override;
	virtual llvm::Value *visit(VariableExprAST &it) override;
	virtual llvm::Value *visit(FileAST &it) override;
	virtual llvm::Value *visit(FunctionAST &it) override;
	virtual llvm::Value *visit(PrototypeAST &it) override;
	virtual llvm::Value *visit(ExprStmt &it) override;
	virtual llvm::Value *visit(ReturnStmt &it) override;
	virtual llvm::Value *visit(IfStmt &it) override;
	virtual llvm::Value *visit(VarDecl &it) override;
	virtual llvm::Value *visit(TypeAST &it) override;
};
} // namespace FoxLang::IR

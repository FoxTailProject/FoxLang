#include "nodes.h"
#include <iostream>

namespace FoxLang {
std::unique_ptr<llvm::LLVMContext> AST::context =
	std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::IRBuilder<>> AST::builder =
	std::make_unique<llvm::IRBuilder<>>(*AST::context);
std::unique_ptr<llvm::Module> AST::llvm_module =
	std::make_unique<llvm::Module>("FoxLang", *AST::context);
std::map<std::string, llvm::Value *> AST::named_values =
	std::map<std::string, llvm::Value *>();

llvm::Value *AST::compile() { return nullptr; }

std::string AST::printName() const { return "unimpl"; }

AST::Exec AST::exec() { return std::monostate{}; }

std::vector<AST *> AST::getChildren() const {
	std::vector<AST *> r;
	return r;
}

llvm::Value *NumberExprAST::compile() {
	return llvm::ConstantInt::get(*context, llvm::APInt(32, atoi(num.c_str())));
}

std::string NumberExprAST::printName() const {
	return fmt::format("NumberExprAST ({})", num);
}

std::string VariableExprAST::printName() const {
	return fmt::format("VariableExprAST ({})", name);
}

std::vector<AST *> BinaryExprAST::getChildren() const {
	std::vector<AST *> r;
	r.push_back(LHS.get());
	r.push_back(RHS.get());
	return r;
}

std::string BinaryExprAST::printName() const {
	return fmt::format("BinaryExprAST ({})", Op.lexeme);
}

llvm::Value *BinaryExprAST::compile() {
	auto left = LHS->compile();
	auto right = RHS->compile();
	if (!left || !right) return nullptr;

	switch (Op.type) {
	case TokenType::PLUS:
		return builder->CreateAdd(left, right);
	case TokenType::MINUS:
		return builder->CreateSub(left, right);
	case TokenType::STAR:
		return builder->CreateMul(left, right);
	case TokenType::LESS:
		return builder->CreateICmpULT(left, right);
	case TokenType::GREATER:
		return builder->CreateICmpUGT(left, right);
	default:
		std::cout << "Unimplemented binary operation '" << Op.lexeme << "'"
				  << std::endl;
		return nullptr;
	}
}

std::vector<AST *> CallExprAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < Args.size(); i++) {
		rets.push_back(Args[i].get());
	}
	return rets;
}

std::string CallExprAST::printName() const {
	return fmt::format("CallExpr ({})", Callee);
}

std::vector<AST *> VarDecl::getChildren() const {
	std::vector<AST *> rets;

	if (value) rets.push_back(value.value().get());
	return rets;
}

std::string VarDecl::printName() const {
	return fmt::format("VarDecl ({})", name);
}

std::vector<AST *> ReturnStmt::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}

std::string ReturnStmt::printName() const { return "Return"; }

std::vector<AST *> ExprStmt::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}

std::string ExprStmt::printName() const { return fmt::format("ExprStmt"); }

std::string TypeAST::printName() const {
	return fmt::format("TypeAST ({})", ident);
}

std::string BlockAST::printName() const { return "BlockAST"; }

std::string PrototypeAST::printName() const {
	return fmt::format("Prototype ({}, {} params)", name, args.size());
}

std::string FunctionAST::printName() const { return "Function"; }

std::string FileAST::printName() const {
	return fmt::format("FileAST ({})", name);
}

std::vector<AST *> BlockAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < content.size(); i++) {
		rets.push_back(content[i].get());
	}
	return rets;
}

std::vector<AST *> PrototypeAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < types.size(); i++) {
		rets.push_back(types[i].get());
	}
	rets.push_back(retType.get());
	return rets;
}

std::vector<AST *> FunctionAST::getChildren() const {
	std::vector<AST *> r;
	r.push_back(proto.get());
	r.push_back(body.get());
	return r;
}

std::vector<AST *> FileAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < expressions.size(); i++) {
		rets.push_back(expressions[i].get());
	}
	return rets;
}
} // namespace FoxLang

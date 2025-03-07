#include "nodes.h"

namespace FoxLang {
llvm::Value *FoxLang::AST::compile() { return nullptr; }

std::string FoxLang::AST::printName() const { return "unimpl"; }

FoxLang::AST::Exec FoxLang::AST::exec() { return std::monostate{}; }

std::vector<AST *> FoxLang::AST::getChildren() const {
	std::vector<AST *> r;
	return r;
}

llvm::Value *FoxLang::NumberExprAST::compile() {
	return llvm::ConstantInt::get(*context, llvm::APInt(32, atoi(num.c_str())));
}

std::string FoxLang::NumberExprAST::printName() const {
	return fmt::format("NumberExprAST ({})", num);
}

std::string FoxLang::VariableExprAST::printName() const {
	return fmt::format("VariableExprAST ({})", name);
}

std::vector<AST *> FoxLang::BinaryExprAST::getChildren() const {
	std::vector<AST *> r;
	r.push_back(LHS.get());
	r.push_back(RHS.get());
	return r;
}

std::string FoxLang::BinaryExprAST::printName() const {
	return fmt::format("BinaryExprAST ({})", Op.lexeme);
}

std::vector<AST *> FoxLang::CallExprAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < Args.size(); i++) {
		rets.push_back(Args[i].get());
	}
	return rets;
}

std::string FoxLang::CallExprAST::printName() const {
	return fmt::format("CallExpr ({})", Callee);
}

std::vector<AST *> FoxLang::VarDecl::getChildren() const {
	std::vector<AST *> rets;

	if (value) rets.push_back(value.value().get());
	return rets;
}

std::string FoxLang::VarDecl::printName() const {
	return fmt::format("VarDecl ({})", name);
}

std::vector<AST *> FoxLang::ReturnStmt::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}

std::string FoxLang::ReturnStmt::printName() const { return "Return"; }

std::vector<AST *> FoxLang::ExprStmt::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}

std::string FoxLang::ExprStmt::printName() const {
	return fmt::format("ExprStmt");
}

std::string FoxLang::TypeAST::printName() const {
	return fmt::format("TypeAST ({})", ident);
}

std::string FoxLang::BlockAST::printName() const { return "BlockAST"; }

std::string FoxLang::PrototypeAST::printName() const {
	return fmt::format("Prototype ({}, {} params)", name, args.size());
}

std::string FoxLang::FunctionAST::printName() const { return "Function"; }

std::string FoxLang::FileAST::printName() const {
	return fmt::format("FileAST ({})", name);
}

std::vector<AST *> FoxLang::BlockAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < content.size(); i++) {
		rets.push_back(content[i].get());
	}
	return rets;
}

std::vector<AST *> FoxLang::PrototypeAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < types.size(); i++) {
		rets.push_back(types[i].get());
	}
	rets.push_back(retType.get());
	return rets;
}

std::vector<AST *> FoxLang::FunctionAST::getChildren() const {
	std::vector<AST *> r;
	r.push_back(proto.get());
	r.push_back(body.get());
	return r;
}

std::vector<AST *> FoxLang::FileAST::getChildren() const {
	std::vector<AST *> rets;

	for (size_t i = 0; i < expressions.size(); i++) {
		rets.push_back(expressions[i].get());
	}
	return rets;
}
} // namespace FoxLang

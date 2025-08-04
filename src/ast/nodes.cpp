#include "nodes.hpp"
#include "../ir/generator.hpp"

#include <iostream>

namespace FoxLang {
std::string AST::printName() const { return "unimpl"; }

AST::Exec AST::exec() { return std::monostate{}; }

std::vector<AST *> AST::getChildren() const {
	std::vector<AST *> r;
	return r;
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

std::string IfStmt::printName() const { return "IfStmt"; }

std::vector<AST *> IfStmt::getChildren() const {
	std::vector<AST *> vec;
	vec.push_back(condition.get());
	vec.push_back(block.get());
	if (else_) vec.push_back(else_.value().get());
	return vec;
}

llvm::Value *BlockAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *BinaryExprAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *CallExprAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *NumberExprAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *VariableExprAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *FileAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *FunctionAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *PrototypeAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *ExprStmt::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *ReturnStmt::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *VarDecl::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *TypeAST::accept(ASTVisitor &v) { return v.visit(*this); }
llvm::Value *IfStmt::accept(ASTVisitor &v) { return v.visit(*this); }
} // namespace FoxLang

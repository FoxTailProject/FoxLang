#include "ast_nodes.hpp"
#include "ir_generator.hpp"

#include <iostream>

namespace FoxLang {
std::string AST::printName() const { return "unimpl"; }

AST::Exec AST::exec() { return std::monostate{}; }

std::vector<AST *> AST::getChildren() const {
	std::vector<AST *> r;
	return r;
}

std::string NumberExprAST::printName() const {
	return fmt::format("NumberExprAST ({})", value);
}

std::string BoolLiteralAST::printName() const {
	return fmt::format("BoolLiteralAST ({})", value);
}

std::string StringLiteralAST::printName() const {
	return fmt::format("StringLiteralAST ({})", value);
}

std::string StructLiteralAST::printName() const {
	return fmt::format("StructLiteralAST ({})", names.size());
}

std::vector<AST *> StructLiteralAST::getChildren() const {
	std::vector<AST *> r;

	for (auto i : values)
		r.push_back(i.get());

	return r;
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
	if (value) r.push_back(value.value().get());
	return r;
}

std::string ReturnStmt::printName() const { return "Return"; }

std::vector<AST *> ExprStmt::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}

std::string ExprStmt::printName() const { return fmt::format("ExprStmt"); }

std::string TypeAST::printName() const { return fmt::format("TypeAST ()"); }
const TypeAST::ctype TypeAST::conversion[] = {
	{.name = "i128", .value = Type::i128},
	{.name = "i64", .value = Type::i64},
	{.name = "i32", .value = Type::i32},
	{.name = "i16", .value = Type::i16},
	{.name = "i8", .value = Type::i8},
	{.name = "u128", .value = Type::u128},
	{.name = "u64", .value = Type::u64},
	{.name = "u32", .value = Type::u32},
	{.name = "u16", .value = Type::u16},
	{.name = "u8", .value = Type::u8},
	{.name = "f128", .value = Type::f128},
	{.name = "f64", .value = Type::f64},
	{.name = "f32", .value = Type::f32},
	{.name = "f16", .value = Type::f16},
	// {.name = "string", .value = Type::string},
	{.name = "bool", .value = Type::_bool},
};

std::vector<AST *> StructMemberAST::getChildren() const {
	std::vector<AST *> r;
	r.push_back(value.get());
	return r;
}
std::string StructMemberAST::printName() const {
	return fmt::format("StructMemberAST");
}

std::vector<AST *> StructAST::getChildren() const {
	std::vector<AST *> r;
	for (auto i : members)
		r.push_back(i.get());
	return r;
}
std::string StructAST::printName() const {
	return fmt::format("StructAST ({})", members.size());
}

std::string BlockAST::printName() const { return "BlockAST"; }

std::vector<AST *> ParameterAST::getChildren() const {
	std::vector<AST *> t;
	return t;
}

std::string ParameterAST::printName() const {
	return fmt::format("Parameter ({})", name);
}

std::string PrototypeAST::printName() const {
	return fmt::format("Prototype ({}, {} params)", name, parameters.size());
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

	for (size_t i = 0; i < parameters.size(); i++) {
		rets.push_back(parameters[i]->type.get());
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

std::string WhileStmt::printName() const { return "WhileStmt"; }

std::vector<AST *> WhileStmt::getChildren() const {
	std::vector<AST *> vec;
	vec.push_back(condition.get());
	vec.push_back(block.get());
	return vec;
}

void BlockAST::accept(ASTVisitor &v) { v.visit(*this); }
void BinaryExprAST::accept(ASTVisitor &v) { v.visit(*this); }
void CallExprAST::accept(ASTVisitor &v) { v.visit(*this); }
void NumberExprAST::accept(ASTVisitor &v) { v.visit(*this); }
void StringLiteralAST::accept(ASTVisitor &v) { v.visit(*this); }
void BoolLiteralAST::accept(ASTVisitor &v) { v.visit(*this); }
void StructLiteralAST::accept(ASTVisitor &v) { v.visit(*this); }
void VariableExprAST::accept(ASTVisitor &v) { v.visit(*this); }
void FileAST::accept(ASTVisitor &v) { v.visit(*this); }
void FunctionAST::accept(ASTVisitor &v) { v.visit(*this); }
void PrototypeAST::accept(ASTVisitor &v) { v.visit(*this); }
void ParameterAST::accept(ASTVisitor &v) { v.visit(*this); }
void ExprStmt::accept(ASTVisitor &v) { v.visit(*this); }
void ReturnStmt::accept(ASTVisitor &v) { v.visit(*this); }
void VarDecl::accept(ASTVisitor &v) { v.visit(*this); }
void TypeAST::accept(ASTVisitor &v) { v.visit(*this); }
void StructAST::accept(ASTVisitor &v) { v.visit(*this); }
void StructMemberAST::accept(ASTVisitor &v) { v.visit(*this); }
void IfStmt::accept(ASTVisitor &v) { v.visit(*this); }
void WhileStmt::accept(ASTVisitor &v) { v.visit(*this); }
} // namespace FoxLang

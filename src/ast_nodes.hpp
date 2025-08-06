#pragma once

#include "tokens.hpp"

#include <cstdlib>
#include <fmt/core.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace FoxLang {
class ASTVisitor;

class AST {
public:
	typedef std::variant<std::monostate, int, float, std::string, bool> Exec;
	virtual ~AST() = default;

	virtual std::vector<AST *> getChildren() const;

	virtual std::string printName() const;

	virtual Exec exec();

	virtual void accept(ASTVisitor &ir) = 0;
};

/// ExprAST - Base class for all expression nodes.
class ExprAST : public AST {};

class StmtAST : public AST {};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
public:
	std::string num;

public:
	explicit NumberExprAST(const std::string &num) : num(num) {}

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
	std::string name;

public:
	explicit VariableExprAST(const std::string &name) : name(name) {}

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
public:
	Token Op;
	std::shared_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(Token Op, std::shared_ptr<ExprAST> LHS,
				  std::shared_ptr<ExprAST> RHS)
		: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;
	void accept(ASTVisitor &ir) override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
public:
	std::string Callee;
	std::vector<std::shared_ptr<ExprAST>> Args;

public:
	CallExprAST(const std::string &Callee,
				std::vector<std::shared_ptr<ExprAST>> Args)
		: Callee(Callee), Args(std::move(Args)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class VarDecl : public StmtAST {
public:
	std::string name;
	std::optional<std::shared_ptr<ExprAST>> value;
	bool mut;

public:
	VarDecl(const std::string &name,
			std::optional<std::shared_ptr<ExprAST>> value, bool mut)
		: name(name), value(std::move(value)), mut(mut) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class ReturnStmt : public StmtAST {
public:
	std::optional<std::shared_ptr<ExprAST>> value;

public:
	ReturnStmt(std::optional<std::shared_ptr<ExprAST>> value)
		: value(std::move(value)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class ExprStmt : public StmtAST {
public:
	std::shared_ptr<ExprAST> value;

public:
	ExprStmt(std::shared_ptr<ExprAST> value) : value(std::move(value)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class Literal : public ExprAST {
public:
	typedef std::variant<std::string, bool> Data;
	Data data;

public:
	Literal(Data data) : data(data) {}

	virtual std::vector<AST *> getChildren() const override;

	virtual std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class TypeAST : public AST {
public:
	std::string ident;

public:
	explicit TypeAST(const std::string &ident) : ident(ident) {}

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class BlockAST : public AST {
public:
	std::vector<std::shared_ptr<StmtAST>> content;
	bool blockless;

public:
	BlockAST(std::vector<std::shared_ptr<StmtAST>> content, bool blockless)
		: content(std::move(content)), blockless(blockless) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class IfStmt : public StmtAST {
public:
	std::shared_ptr<ExprAST> condition;
	std::shared_ptr<BlockAST> block;
	std::optional<std::shared_ptr<BlockAST>> else_;

public:
	IfStmt(std::shared_ptr<ExprAST> condition, std::shared_ptr<BlockAST> block,
		   std::optional<std::shared_ptr<BlockAST>> else_)
		: condition(condition), block(block), else_(else_) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class WhileStmt : public StmtAST {
public:
	std::shared_ptr<ExprAST> condition;
	std::shared_ptr<BlockAST> block;

public:
	WhileStmt(std::shared_ptr<ExprAST> condition,
			  std::shared_ptr<BlockAST> block)
		: condition(condition), block(block) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public AST {
public:
	std::string name;
	std::vector<std::string> args;
	std::vector<std::shared_ptr<TypeAST>> types;
	std::shared_ptr<TypeAST> retType;

public:
	PrototypeAST(const std::string &name, std::vector<std::string> args,
				 std::vector<std::shared_ptr<TypeAST>> types,
				 std::shared_ptr<TypeAST> retType)
		: name(name), args(args), types(types), retType(std::move(retType)) {}

	const std::string &getName() const { return name; }

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public AST {
public:
	std::shared_ptr<PrototypeAST> proto;
	std::shared_ptr<BlockAST> body;

public:
	FunctionAST(std::shared_ptr<PrototypeAST> proto,
				std::shared_ptr<BlockAST> body)
		: proto(proto), body(body) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class FileAST : public AST {
public:
	std::string name;
	std::vector<std::shared_ptr<AST>> expressions;

public:
	FileAST(const std::string &name,
			std::vector<std::shared_ptr<AST>> &&expressions)
		: name(name), expressions(std::move(expressions)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	Exec exec() override {
		for (auto i : expressions) {
			if (dynamic_cast<FunctionAST *>(i.get()) &&
				((FunctionAST *)i.get())->proto->name == "main") {
				i->exec();
				return std::monostate{};
			}
		}
		return std::monostate{};
	}

	void accept(ASTVisitor &ir) override;
};

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
inline int getPrecedence(TokenType token) {
	// Make sure it's a declared binop.
	switch (token) {
	case TokenType::AND:
		return 20;
	case TokenType::OR:
		return 30;
	case TokenType::LESS:
	case TokenType::LESS_EQUAL:
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
		return 40;
	case TokenType::MINUS:
	case TokenType::PLUS:
		return 50;
	case TokenType::STAR:
	case TokenType::SLASH:
		return 60;
	default:
		return -1;
	}
}

namespace IR {}
} // namespace FoxLang

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

	llvm::Value *llvm_value;

	virtual std::vector<AST *> getChildren() const;

	virtual std::string printName() const;

	virtual Exec exec();

	virtual void accept(ASTVisitor &ir) = 0;
};

class ExprAST : public AST {};

class StmtAST : public AST {};

class Literal : public ExprAST {};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public Literal {
public:
	std::string value;

public:
	explicit NumberExprAST(const std::string &value) : value(value) {}

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class StringLiteralAST : public Literal {
public:
	std::string value;

public:
	StringLiteralAST(const std::string &value) : value(value) {}

	virtual std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class BoolLiteralAST : public Literal {
public:
	bool value;

public:
	BoolLiteralAST(const bool value) : value(value) {}

	virtual std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class TypeAST;
class StructLiteralAST : public Literal {
public:
	std::vector<std::string> names;
	std::vector<std::shared_ptr<ExprAST>> values;
	TypeAST *type;

public:
	StructLiteralAST(std::vector<std::string> names,
					 std::vector<std::shared_ptr<ExprAST>> values)
		: names(names), values(values) {}

	virtual std::string printName() const override;
	virtual std::vector<AST *> getChildren() const override;

	void accept(ASTVisitor &ir) override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
	std::string name;
	AST *resolved_name;

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
	AST *resolved_name;

public:
	CallExprAST(const std::string &Callee,
				std::vector<std::shared_ptr<ExprAST>> Args)
		: Callee(Callee), Args(std::move(Args)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;
};

class StructAST;
class TypeAST : public AST {
public:
	// std::string ident;
	enum class Type {
		i128 = 2,
		i64,
		i32,
		i16,
		i8,
		u128,
		u64,
		u32,
		u16,
		u8,
		f128,
		f64,
		f32,
		f16,
		// string,
		_bool,
		_struct,
		array,
		pointer,
		__int,	// __: for internal use only, for unsized literals
		__uint, // can implicitly cast to __int
		__float,
	};
	Type type;
	std::optional<std::shared_ptr<TypeAST>> child;
	std::string data;
	StructAST *resolved_name;

	typedef struct {
		std::string name;
		Type value;
	} ctype;
	const static ctype conversion[16];

public:
	explicit TypeAST(const Type &type,
					 std::optional<std::shared_ptr<TypeAST>> child,
					 std::string data)
		: type(type), child(child), data(data) {}

	std::string printName() const override;

	void accept(ASTVisitor &ir) override;

	inline bool operator==(const TypeAST &rhs) const {
		if (type != rhs.type) return false;

		if (type != Type::pointer && type != Type::array &&
			type != Type::_struct)
			return true;

		if (type == Type::_struct) return resolved_name == rhs.resolved_name;

		// both ptr and arr only have a child (no arr len rn)
		// so simple child comparison
		return *child.value().get() == *rhs.child.value().get();
	}
	inline bool operator!=(const TypeAST &rhs) const { return !(*this == rhs); }
};

class VarDecl : public StmtAST {
public:
	std::string name;
	std::shared_ptr<TypeAST> type;
	std::optional<std::shared_ptr<ExprAST>> value;
	bool mut;

public:
	VarDecl(const std::string &name, std::shared_ptr<TypeAST> type,
			std::optional<std::shared_ptr<ExprAST>> value, bool mut)
		: name(name), type(std::move(type)), value(std::move(value)), mut(mut) {
	}

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

// i know that its better to not have this as a seperate class and have the
// struct manage it, but this makes things like name res easier
class StructMemberAST : public AST {
public:
	std::string name;
	std::shared_ptr<TypeAST> value;
	llvm::Value *llvm_value;

public:
	StructMemberAST(std::string name, std::shared_ptr<TypeAST> value)
		: name(name), value(value) {}

	std::vector<AST *> getChildren() const override;
	std::string printName() const override;
	void accept(ASTVisitor &ir) override;
};

class StructAST : public AST {
public:
	std::string name;
	std::vector<std::shared_ptr<StructMemberAST>> members;
	llvm::StructType *llvm_value;

public:
	StructAST(std::string name,
			  std::vector<std::shared_ptr<StructMemberAST>> members)
		: name(name), members(members) {}

	std::vector<AST *> getChildren() const override;
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

/// ParameteAST - Represents the paramteres for a function to ease the process
/// of name resolution and ir generation
class ParameterAST : public AST {
public:
	std::string name;
	std::shared_ptr<TypeAST> type;

public:
	ParameterAST(std::string name, std::shared_ptr<TypeAST> type)
		: name(name), type(type) {}

	std::vector<AST *> getChildren() const override;
	std::string printName() const override;
	void accept(ASTVisitor &v) override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public AST {
public:
	std::string name;
	std::vector<std::shared_ptr<ParameterAST>> parameters;
	std::shared_ptr<TypeAST> retType;

public:
	PrototypeAST(const std::string &name,
				 std::vector<std::shared_ptr<ParameterAST>> parameters,
				 std::shared_ptr<TypeAST> retType)
		: name(name), parameters(parameters), retType(std::move(retType)) {}

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

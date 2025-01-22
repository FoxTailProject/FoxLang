#pragma once

#include "../lexer/tokens.hpp"

#include <memory>
#include <string>
#include <vector>

namespace FoxLang {
class AST {
public:
	virtual ~AST() = default;
};

/// ExprAST - Base class for all expression nodes.
class ExprAST : AST {};

class StmtAST : AST {};

class FileAST : public AST {

	std::string name;
	std::vector<std::unique_ptr<AST>> expressions;

public:
	FileAST(const std::string &name,
			std::vector<std::unique_ptr<AST>> &&expressions)
		: name(name), expressions(std::move(expressions)) {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
	std::string Val;

public:
	explicit NumberExprAST(const std::string &Val) : Val(Val) {}
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
	std::string Name;

public:
	explicit VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
	Token Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(Token Op, std::unique_ptr<ExprAST> LHS,
				  std::unique_ptr<ExprAST> RHS)
		: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;

public:
	CallExprAST(const std::string &Callee,
				std::vector<std::unique_ptr<ExprAST>> Args)
		: Callee(Callee), Args(std::move(Args)) {}
};

class VarDecl : public StmtAST {
	std::string name;
	std::optional<std::unique_ptr<ExprAST>> value;
	bool mut;

public:
	VarDecl(const std::string &name,
			std::optional<std::unique_ptr<ExprAST>> value, bool mut)
		: name(name), value(std::move(value)), mut(mut) {}
};

class ExprStmt : public StmtAST {
	std::unique_ptr<ExprAST> value;

public:
	ExprStmt(std::unique_ptr<ExprAST> value) : value(std::move(value)) {}
};

class TypeAST : public AST {

	std::string ident;

public:
	explicit TypeAST(const std::string &ident) : ident(ident) {}
};

class BlockAST : public AST {
	std::vector<std::unique_ptr<StmtAST>> content;

public:
	BlockAST(std::vector<std::unique_ptr<StmtAST>> content)
		: content(std::move(content)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : public AST {

	std::string Name;
	std::vector<std::string> Args;
	std::vector<std::unique_ptr<TypeAST>> Types;
	std::unique_ptr<TypeAST> retType;

public:
	PrototypeAST(const std::string &Name, std::vector<std::string> Args,
				 std::vector<std::unique_ptr<TypeAST>> Types,
				 std::unique_ptr<TypeAST> retType)
		: Name(Name), Args(std::move(Args)), Types(std::move(Types)),
		  retType(std::move(retType)) {}

	const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public AST {
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<BlockAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
				std::unique_ptr<BlockAST> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}
};

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
inline int getPrecedence(TokenType token) {
	// Make sure it's a declared binop.
	switch (token) {
	case TokenType::LESS:
		return 10;
	case TokenType::MINUS:
	case TokenType::PLUS:
		return 20;
	case TokenType::STAR:
	case TokenType::SLASH:
		return 40;
	default:
		return -1;
	}
}

/// LogError* - These are little helper functions for error handling.
inline std::unique_ptr<ExprAST> LogError(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

inline std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
	LogError(Str);
	return nullptr;
}

} // namespace FoxLang

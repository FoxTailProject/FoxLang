#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../tokenstreamer.hpp"

namespace FoxLang {

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
	virtual ~ExprAST() = default;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
	double Val;

public:
	NumberExprAST(double Val) : Val(Val) {}
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
	std::string Name;

public:
	VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
	char Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
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

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
	std::string Name;
	std::vector<std::string> Args;

public:
	PrototypeAST(const std::string &Name, std::vector<std::string> Args)
		: Name(Name), Args(std::move(Args)) {}

	const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
	std::unique_ptr<PrototypeAST> Proto;
	std::unique_ptr<ExprAST> Body;

public:
	FunctionAST(std::unique_ptr<PrototypeAST> Proto,
				std::unique_ptr<ExprAST> Body)
		: Proto(std::move(Proto)), Body(std::move(Body)) {}
};

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
	if (!isascii(CurTok)) return -1;

	// Make sure it's a declared binop.
	int TokPrec = -1;
	switch (CurTok) {
	default:
		break;
	}
	if (TokPrec <= 0) return -1;
	return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
	LogError(Str);
	return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr(TokenStreamer *streamer) {
	auto Result = std::make_unique<NumberExprAST>(NumVal);
	streamer->peek(); // consume the number
	return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr(TokenStreamer *streamer) {
	streamer->peek(); // eat (.
	auto V = ParseExpression();
	if (!V) return nullptr;

	if (CurTok != ')') return LogError("expected ')'");
	streamer->peek(); // eat ).
	return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr(TokenStreamer *streamer) {
	std::string IdName = IdentifierStr;

	streamer->peek(); // eat identifier.

	if (CurTok != '(') // Simple variable ref.
		return std::make_unique<VariableExprAST>(IdName);

	// Call.
	streamer->peek(); // eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (true) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')') break;

			if (CurTok != ',')
				return LogError("Expected ')' or ',' in argument list");
			streamer->peek();
		}
	}

	// Eat the ')'.
	streamer->peek();

	return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary(TokenStreamer *streamer) {
	auto token = streamer->peek();
	switch (token.type) {
	default:
		return LogError("unknown token when expecting an expression");
	case TokenType::IDENTIFIER:
		return ParseIdentifierExpr();
	case TokenType::NUMBER:
		return ParseNumberExpr();
	case TokenType::LEFT_PAREN:
		return ParseParenExpr();
	}
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(TokenStreamer *streamer,
											  int ExprPrec,
											  std::unique_ptr<ExprAST> LHS) {
	// If this is a binop, find its precedence.
	while (true) {
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current
		// binop, consume it, otherwise we are done.
		if (TokPrec < ExprPrec) return LHS;

		// Okay, we know this is a binop.
		Token BinOp = streamer->peek();

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimary();
		if (!RHS) return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinOpRHS(streamer, TokPrec + 1, std::move(RHS));
			if (!RHS) return nullptr;
		}

		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
											  std::move(RHS));
	}
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
	auto LHS = ParsePrimary();
	if (!LHS) return nullptr;

	return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype(TokenStreamer *streamer) {
	auto token = streamer->peek();
	if (token.type != TokenType::IDENTIFIER)
		return LogErrorP("Expected function name in prototype");

	std::string FnName = token.lexeme;
	token = streamer->peek();

	if (token.type != TokenType::LEFT_PAREN)
		return LogErrorP("Expected '(' in prototype");

	std::vector<std::string> ArgNames;
	token = streamer->peek();
	while (token.type != TokenType::RIGHT_PAREN) {
		if (token.type == TokenType::IDENTIFIER)
			ArgNames.push_back(token.lexeme);
		else if (token.type != TokenType::COMMA)
			return LogErrorP("Expected Comma seperated list of arguments");
		token = streamer->peek();
	}
	if (token.type != TokenType::RIGHT_PAREN)
		return LogErrorP("Expected ')' in prototype");

	// success.
	streamer->peek(); // eat ')'.

	return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition(TokenStreamer *streamer) {
	streamer->peek(); // eat def.
	auto Proto = ParsePrototype();
	if (!Proto) return nullptr;

	if (auto E = ParseExpression())
		return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	return nullptr;
}

/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
	if (auto E = ParseExpression()) {
		// Make an anonymous proto.
		auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
													std::vector<std::string>());
		return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
	}
	return nullptr;
}

/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern(TokenStreamer *streamer) {
	streamer->peek(); // eat extern.
	return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition(TokenStreamer *streamer) {
	if (ParseDefinition()) {
		fprintf(stderr, "Parsed a function definition.\n");
	} else {
		// Skip token for error recovery.
		streamer->peek();
	}
}

static void HandleExtern(TokenStreamer *streamer) {
	if (ParseExtern()) {
		fprintf(stderr, "Parsed an extern\n");
	} else {
		// Skip token for error recovery.
		streamer->peek();
	}
}

void HandleTopLevelExpression(TokenStreamer *streamer) {
	// Evaluate a top-level expression into an anonymous function.
	if (ParseTopLevelExpr()) {
		fprintf(stderr, "Parsed a top-level expr\n");
	} else {
		// Skip token for error recovery.
		streamer->peek();
	}
}

/// top ::= definition | external | expression | ';'
void MainLoop(TokenStreamer *streamer) {
	while (true) {
		auto token = streamer->peek();
		switch (token.type) {
		case TokenType::EOF_TOKEN:
			return;
		case TokenType::SEMICOLON: // ignore top-level semicolons.
			streamer->peek();
			break;
		case TokenType::FUNC:
			HandleDefinition();
			break;
		case TokenType::EXTERN:
			HandleExtern();
			break;
		default:
			HandleTopLevelExpression();
			break;
		}
	}
}
} // namespace FoxLang

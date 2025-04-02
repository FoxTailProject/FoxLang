#pragma once

#include "../lexer/tokens.h"
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
class AST {
public:
	typedef std::variant<std::monostate, int, float, std::string, bool> Exec;
	virtual ~AST() = default;

	virtual std::vector<AST *> getChildren() const;

	virtual std::string printName() const;

	virtual Exec exec();

	virtual llvm::Value *compile();

	static std::unique_ptr<llvm::LLVMContext> context;
	static std::unique_ptr<llvm::IRBuilder<>> builder;
	static std::unique_ptr<llvm::Module> llvm_module;

	static std::map<std::string, llvm::Value *> named_values;
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

	llvm::Value *compile() override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
	std::string name;

public:
	explicit VariableExprAST(const std::string &name) : name(name) {}

	std::string printName() const override;

	llvm::Value *compile() override {
		if (named_values.count(name))
			return named_values[name];
		else {
			fmt::println("Invalid use of undefined identifier '{}'", name);
			return nullptr;
		}
	}
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
	llvm::Value *compile() override;
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

	llvm::Value *compile() override {
		llvm::Function *callee = llvm_module->getFunction(Callee);
		if (!callee) {
			// TODO: Generate a new function prototype
			// If it doesn't have a body at the end of compilation
			// then it throws an error
			std::cout << "Could not find function '" << Callee << "'"
					  << std::endl;
			return nullptr;
		}

		if (callee->arg_size() != Args.size()) {
			std::cout << "Call and fuction parity do not match" << std::endl;
			return nullptr;
		}

		std::vector<llvm::Value *> args(Args.size());
		for (int i = 0, e = Args.size(); i < e; i++) {
			auto a = Args[i]->compile();
			if (!a) {
				std::cout << "Unable to compile argument" << std::endl;
				return nullptr;
			}
			args[i] = a;
		}

		return builder->CreateCall(callee, args);
	}
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

	llvm::Value *compile() override {
		if (!mut) {
			if (!value) return nullptr;
			named_values[name] = value.value()->compile();
			return nullptr;
		}

		std::cout << "unimpl" << std::endl;
		return nullptr;
	}
};

class ReturnStmt : public StmtAST {
public:
	std::shared_ptr<ExprAST> value;

public:
	ReturnStmt(std::shared_ptr<ExprAST> value) : value(std::move(value)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	llvm::Value *compile() override {
		builder->CreateRet(value->compile());
		return nullptr;
	}
};

class ExprStmt : public StmtAST {
public:
	std::shared_ptr<ExprAST> value;

public:
	ExprStmt(std::shared_ptr<ExprAST> value) : value(std::move(value)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	llvm::Value *compile() override {
		value->compile();
		return nullptr;
	}
};

class TypeAST : public AST {
public:
	std::string ident;

public:
	explicit TypeAST(const std::string &ident) : ident(ident) {}

	std::string printName() const override;
};

class BlockAST : public AST {
public:
	std::vector<std::shared_ptr<StmtAST>> content;

public:
	BlockAST(std::vector<std::shared_ptr<StmtAST>> content)
		: content(std::move(content)) {}

	std::vector<AST *> getChildren() const override;

	std::string printName() const override;

	llvm::Value *compile() override {
		for (auto stmt : content) {
			stmt->compile();
		}

		return nullptr;
	}
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

	llvm::Function *compile() override {
		std::vector<llvm::Type *> ints(args.size(),
									   llvm::Type::getInt32Ty(*context));
		llvm::FunctionType *ft = llvm::FunctionType::get(
			llvm::Type::getInt32Ty(*context), ints, false);
		llvm::Function *f = llvm::Function::Create(
			ft, llvm::Function::ExternalLinkage, name, llvm_module.get());

		int i = 0;
		for (auto &arg : f->args())
			arg.setName(args[i++]);
		return f;
	}
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

	llvm::Function *compile() override {
		llvm::Function *func = llvm_module->getFunction(proto->name);

		if (!func) func = proto->compile();
		if (!func) return nullptr;

		if (!func->empty()) {
			std::cout << "Cannot redefine function '" << proto->name << "'"
					  << std::endl;
			return nullptr;
		}

		llvm::BasicBlock *bodyBlock =
			llvm::BasicBlock::Create(*context, "entry", func);
		builder->SetInsertPoint(bodyBlock);

		named_values.clear();
		auto c = func->arg_size();
		auto a = proto->args;
		for (int i = 0; i < c; ++i) {
			named_values[a[i]] = (func->arg_begin() + i);
		}

		body->compile();

		llvm::verifyFunction(*func);
		return func;
	}
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

	llvm::Value *compile() override {
		for (auto expr : expressions) {
			expr->compile();
		}

		llvm_module->print(llvm::errs(), nullptr);
		return nullptr;
	}
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
inline std::shared_ptr<ExprAST> LogError(const char *Str) {
	fprintf(stderr, "Error: %s\n", Str);
	return nullptr;
}

inline std::shared_ptr<PrototypeAST> LogErrorP(const char *Str) {
	LogError(Str);
	return nullptr;
}

} // namespace FoxLang

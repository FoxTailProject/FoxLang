#include "ir_generator.hpp"
#include <fmt/base.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <variant>

namespace FoxLang::IR {
std::unique_ptr<llvm::LLVMContext> Generator::context =
	std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::IRBuilder<>> Generator::builder =
	std::make_unique<llvm::IRBuilder<>>(*Generator::context);
std::unique_ptr<llvm::Module> Generator::llvm_module =
	std::make_unique<llvm::Module>("FoxLang", *Generator::context);

void Generator::visit(BlockAST &it) {
	for (auto stmt : it.content) {
		stmt->accept(*this);
		// visit(stmt);
	}

	return;
}

void Generator::visit(BinaryExprAST &it) {
	it.LHS->accept(*this);
	auto left = returned;
	it.RHS->accept(*this);
	auto right = returned;
	if (!left || !right) return;

	switch (it.Op.type) {
	case TokenType::PLUS: {
		returned = builder->CreateAdd(left, right);
		return;
	}
	case TokenType::MINUS:
		returned = builder->CreateSub(left, right);
		return;
	case TokenType::STAR:
		returned = builder->CreateMul(left, right);
		return;
	case TokenType::LESS:
		returned = builder->CreateICmpULT(left, right);
		return;
	case TokenType::GREATER:
		returned = builder->CreateICmpUGT(left, right);
		return;
	default:
		std::cout << "Unimplemented binary operation '" << it.Op.lexeme << "'"
				  << std::endl;
		return;
	}
}

void Generator::visit(CallExprAST &it) {
	llvm::Function *callee = llvm_module->getFunction(it.Callee);
	if (!callee) {
		// TODO: Generate a new function prototype
		// If it doesn't have a body at the end of compilation
		// then it throws an error
		std::cout << "Could not find function '" << it.Callee << "'"
				  << std::endl;
		return;
	}

	if (callee->arg_size() != it.Args.size()) {
		std::cout << "Call and fuction parity do not match" << std::endl;
		return;
	}

	std::vector<llvm::Value *> args(it.Args.size());
	for (int i = 0, e = it.Args.size(); i < e; i++) {
		it.Args[i]->accept(*this);
		auto a = returned;
		if (!a) {
			std::cout << "Unable to compile argument" << std::endl;
			return;
		}
		args[i] = a;
	}

	returned = builder->CreateCall(callee, args);
}

void Generator::visit(NumberExprAST &it) {
	returned =
		llvm::ConstantInt::get(*context, llvm::APInt(32, atoi(it.num.c_str())));
}

void Generator::visit(VariableExprAST &it) {
	std::cout << it.name << std::endl;
	returned = it.resolved_name->llvm_value;
}

void breadth_function_define(FunctionAST *, Generator &);
void Generator::visit(FileAST &it) {
	std::cout << "file time baby!\n";
	for (auto child : it.getChildren()) {
		FunctionAST *c = dynamic_cast<FunctionAST *>(child);
		if (c != nullptr) breadth_function_define(c, *this);
	}

	for (auto child : it.getChildren()) {
		child->accept(*this);
	}

	llvm::verifyModule(*llvm_module);
	llvm_module->print(llvm::errs(), nullptr);
	return;
}

void Generator::visit(FunctionAST &it) {
	llvm::Function *func = llvm_module->getFunction(it.proto->name);

	if (!func) {
		it.proto->accept(*this);
		func = (llvm::Function *)returned;
	}
	if (!func) return;

	if (!func->empty()) {
		std::cout << "Cannot redefine function '" << it.proto->name << "'"
				  << std::endl;
		return;
	}

	llvm::BasicBlock *bodyBlock =
		llvm::BasicBlock::Create(*context, "entry", func);
	builder->SetInsertPoint(bodyBlock);

	it.body->accept(*this);

	llvm::verifyFunction(*func);
	returned = func;
}

// not called due to the breadth pass
void Generator::visit(PrototypeAST &) {}
void Generator::visit(ParameterAST &) {}

void Generator::visit(ReturnStmt &it) {
	if (it.value) {
		it.value.value()->accept(*this);
		builder->CreateRet(returned);
	} else
		builder->CreateRetVoid();

	return;
}

void Generator::visit(IfStmt &it) {
	it.condition->accept(*this);
	auto cond = returned;
	auto function = builder->GetInsertBlock()->getParent();
	auto true_block = llvm::BasicBlock::Create(*context, "if_true", function);

	auto ip = builder->GetInsertBlock();

	// both the if and else block need to be filled in before the return block
	// is created, otherwise if there is an if statement inside an if those
	// blocks will be inserted after the final block, which we dont want
	if (!it.else_.has_value()) {
		builder->SetInsertPoint(true_block);
		it.block->accept(*this);

		auto final_block =
			llvm::BasicBlock::Create(*context, "if_return", function);

		builder->CreateBr(final_block);

		builder->SetInsertPoint(ip);
		builder->CreateCondBr(cond, true_block, final_block);

		builder->SetInsertPoint(final_block);
		return;
	}

	builder->SetInsertPoint(true_block);
	it.block->accept(*this);

	llvm::BasicBlock *false_block =
		llvm::BasicBlock::Create(*context, "if_else", function);

	builder->SetInsertPoint(false_block);
	it.else_.value()->accept(*this);

	auto final_block =
		llvm::BasicBlock::Create(*context, "if_return", function);

	builder->CreateBr(final_block);

	builder->SetInsertPoint(true_block);
	builder->CreateBr(final_block);

	builder->SetInsertPoint(ip);
	builder->CreateCondBr(cond, true_block, false_block);

	builder->SetInsertPoint(final_block);

	return;
}

void Generator::visit(WhileStmt &it) {
	auto func = builder->GetInsertBlock()->getParent();
	auto cond_block = llvm::BasicBlock::Create(*context, "while_cond", func);

	builder->CreateBr(cond_block);

	auto block = llvm::BasicBlock::Create(*context, "while_block", func);
	builder->SetInsertPoint(block);
	it.block->accept(*this);
	builder->CreateBr(cond_block);

	builder->SetInsertPoint(cond_block);
	it.condition->accept(*this);
	auto cond = returned;
	auto end = llvm::BasicBlock::Create(*context, "while_return", func);
	builder->CreateCondBr(cond, block, end);
	builder->SetInsertPoint(end);

	return;
}

void Generator::visit(VarDecl &it) {
	if (!it.value) return;
	it.value.value()->accept(*this);
	llvm::Value *tmp = returned;

	if (!it.mut) {
		tmp->setName(it.name);
		it.llvm_value = tmp;
		returned = tmp;
	}

	auto alloca =
		builder->CreateAlloca(llvm::Type::getInt32Ty(*context), tmp, it.name);
	it.llvm_value = alloca;
	returned = alloca;
}

void Generator::visit(TypeAST &) {}
void Generator::visit(StructAST &it) {}

void Generator::visit(ExprStmt &it) {
	it.value->accept(*this);
	return;
}

template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};
void Generator::visit(Literal &it) {
	returned = std::visit(
		overloads{
			[](std::string &str) {
				return (llvm::Value *)builder->CreateGlobalString(str);
			},
			[](bool &val) {
				return val ? (llvm::Value *)llvm::ConstantInt::getTrue(*context)
						   : (llvm::Value *)llvm::ConstantInt::getFalse(
								 *context);
			},
		},
		it.data);
}

void breadth_function_define(FunctionAST *it, Generator &gen) {
	std::vector<llvm::Type *> ints(it->proto->parameters.size(),
								   llvm::Type::getInt32Ty(*gen.context));
	llvm::FunctionType *ft = llvm::FunctionType::get(
		llvm::Type::getInt32Ty(*gen.context), ints, false);
	llvm::Function *f =
		llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
							   it->proto->name, gen.llvm_module.get());

	int i = 0;
	for (auto &arg : f->args()) {
		arg.setName(it->proto->parameters[i]->name);
		it->proto->parameters[i++]->llvm_value = &arg;
	}

	it->llvm_value = f;
}
} // namespace FoxLang::IR

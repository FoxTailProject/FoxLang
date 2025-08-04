#include "generator.hpp"
#include <fmt/base.h>
#include <llvm/IR/Verifier.h>

namespace FoxLang::IR {
std::unique_ptr<llvm::LLVMContext> Generator::context =
	std::make_unique<llvm::LLVMContext>();
std::unique_ptr<llvm::IRBuilder<>> Generator::builder =
	std::make_unique<llvm::IRBuilder<>>(*Generator::context);
std::unique_ptr<llvm::Module> Generator::llvm_module =
	std::make_unique<llvm::Module>("FoxLang", *Generator::context);
std::map<std::string, llvm::Value *> Generator::named_values =
	std::map<std::string, llvm::Value *>();

llvm::Value *Generator::visit(BlockAST &it) {
	for (auto stmt : it.content) {
		stmt->accept(*this);
	}

	return nullptr;
}

llvm::Value *Generator::visit(BinaryExprAST &it) {
	auto left = it.LHS->accept(*this);
	auto right = it.RHS->accept(*this);
	if (!left || !right) return nullptr;

	switch (it.Op.type) {
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
		std::cout << "Unimplemented binary operation '" << it.Op.lexeme << "'"
				  << std::endl;
		return nullptr;
	}
}

llvm::Value *Generator::visit(CallExprAST &it) {
	llvm::Function *callee = llvm_module->getFunction(it.Callee);
	if (!callee) {
		// TODO: Generate a new function prototype
		// If it doesn't have a body at the end of compilation
		// then it throws an error
		std::cout << "Could not find function '" << it.Callee << "'"
				  << std::endl;
		return nullptr;
	}

	if (callee->arg_size() != it.Args.size()) {
		std::cout << "Call and fuction parity do not match" << std::endl;
		return nullptr;
	}

	std::vector<llvm::Value *> args(it.Args.size());
	for (int i = 0, e = it.Args.size(); i < e; i++) {
		auto a = it.Args[i]->accept(*this);
		if (!a) {
			std::cout << "Unable to compile argument" << std::endl;
			return nullptr;
		}
		args[i] = a;
	}

	return builder->CreateCall(callee, args);
}

llvm::Value *Generator::visit(NumberExprAST &it) {
	return llvm::ConstantInt::get(*context,
								  llvm::APInt(32, atoi(it.num.c_str())));
}

llvm::Value *Generator::visit(VariableExprAST &it) {
	if (named_values.count(it.name))
		return named_values[it.name];
	else {
		return nullptr;
	}
}

llvm::Value *Generator::visit(FileAST &it) {
	std::cout << "file time baby!\n";
	for (auto child : it.getChildren()) {
		child->accept(*this);
	}

	llvm::verifyModule(*llvm_module);
	llvm_module->print(llvm::errs(), nullptr);
	return nullptr;
}

llvm::Value *Generator::visit(FunctionAST &it) {
	llvm::Function *func = llvm_module->getFunction(it.proto->name);

	if (!func) func = (llvm::Function *)it.proto->accept(*this);
	if (!func) return nullptr;

	if (!func->empty()) {
		std::cout << "Cannot redefine function '" << it.proto->name << "'"
				  << std::endl;
		return nullptr;
	}

	llvm::BasicBlock *bodyBlock =
		llvm::BasicBlock::Create(*context, "entry", func);
	builder->SetInsertPoint(bodyBlock);

	named_values.clear();
	auto c = func->arg_size();
	auto a = it.proto->args;
	for (size_t i = 0; i < c; ++i) {
		named_values[a[i]] = (func->arg_begin() + i);
	}

	it.body->accept(*this);

	llvm::verifyFunction(*func);
	return func;
}

llvm::Value *Generator::visit(PrototypeAST &it) {
	std::vector<llvm::Type *> ints(it.args.size(),
								   llvm::Type::getInt32Ty(*context));
	llvm::FunctionType *ft =
		llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), ints, false);
	llvm::Function *f = llvm::Function::Create(
		ft, llvm::Function::ExternalLinkage, it.name, llvm_module.get());

	int i = 0;
	for (auto &arg : f->args())
		arg.setName(it.args[i++]);
	return f;
}

llvm::Value *Generator::visit(ReturnStmt &it) {
	builder->CreateRet(it.value->accept(*this));
	return nullptr;
}

llvm::Value *Generator::visit(IfStmt &it) {
	auto cond = it.condition->accept(*this);
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
		return nullptr;
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

	return nullptr;
}

llvm::Value *Generator::visit(VarDecl &it) {
	if (!it.value) return nullptr;
	llvm::Value *tmp = it.value.value()->accept(*this);

	if (!it.mut) {
		tmp->setName(it.name);
		named_values[it.name] = tmp;
		return tmp;
	}

	auto alloca =
		builder->CreateAlloca(llvm::Type::getInt32Ty(*context), tmp, it.name);
	named_values[it.name] = alloca;
	return alloca;
}

llvm::Value *Generator::visit(TypeAST &it) { return nullptr; }

llvm::Value *Generator::visit(ExprStmt &it) {
	it.value->accept(*this);
	return nullptr;
}

} // namespace FoxLang::IR

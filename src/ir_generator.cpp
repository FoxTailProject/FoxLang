#include "ir_generator.hpp"
#include "ast_nodes.hpp"
#include <fmt/base.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
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
	returned = llvm::ConstantInt::get(*context,
									  llvm::APInt(32, atoi(it.value.c_str())));
}

void Generator::visit(StringLiteralAST &it) {}
void Generator::visit(BoolLiteralAST &it) {}
void Generator::visit(StructLiteralAST &it) {}

void Generator::visit(VariableExprAST &it) {
	std::cout << it.name << std::endl;
	returned = it.resolved_name->llvm_value;
}

void breadth_function_define(FunctionAST *, Generator &);
void breadth_struct_define(StructAST *, Generator &);
void Generator::visit(FileAST &it) {
	std::cout << "file time baby!\n";
	// need to do a struct pass then function pass because functions can return
	// a struct that has not been defined yet
	for (auto child : it.getChildren()) {
		if (auto s = dynamic_cast<StructAST *>(child))
			breadth_struct_define(s, *this);
	}
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
	it.type->accept(*this);
	llvm::Type *type = returned_type;

	if (it.mut) {
		auto alloca = builder->CreateAlloca(type, nullptr, it.name);

		if (it.value) {
			it.value.value()->accept(*this);
			builder->CreateStore(returned, alloca);
		}

		it.llvm_value = alloca;
		returned = alloca;
		return;
	}

	if (!it.value) return;
	it.value.value()->accept(*this);
	llvm::Value *tmp = returned;

	tmp->setName(it.name);
	it.llvm_value = tmp;
	returned = tmp;
}

void Generator::visit(TypeAST &it) {
	using T = TypeAST::Type;
	switch (it.type) {
	case T::i128:
	case T::u128:
		returned_type = llvm::IntegerType::get(*context, 128);
		break;
	case T::i64:
	case T::u64:
		returned_type = llvm::IntegerType::get(*context, 64);
		break;
	case T::i32:
	case T::u32:
		returned_type = llvm::IntegerType::get(*context, 32);
		break;
	case T::i16:
	case T::u16:
		returned_type = llvm::IntegerType::get(*context, 16);
		break;
	case T::i8:
	case T::u8:
		returned_type = llvm::IntegerType::get(*context, 8);
		break;
	case T::f128:
		returned_type = llvm::Type::getFP128Ty(*context);
		break;
	case T::f64:
		returned_type = llvm::Type::getDoubleTy(*context);
		break;
	case T::f32:
		returned_type = llvm::Type::getFloatTy(*context);
		break;
	case T::f16:
		returned_type = llvm::Type::getHalfTy(*context);
		break;
	case T::_bool:
		returned_type = llvm::Type::getInt1Ty(*context);
		break;
	case T::_struct:
		returned_type = it.resolved_name->llvm_value;
		break;
	case T::pointer: {
		it.child.value()->accept(*this);
		returned_type = llvm::PointerType::get(returned_type, 0);
	} break;
	}
}
void Generator::visit(StructAST &it) {
	std::vector<llvm::Type *> types;

	for (auto i : it.members) {
		i->accept(*this);
		types.push_back(returned_type);
	}

	it.llvm_value->setBody(types);
}

void Generator::visit(StructMemberAST &it) { it.value->accept(*this); }

void Generator::visit(ExprStmt &it) {
	it.value->accept(*this);
	return;
}

// template <class... Ts> struct overloads : Ts... {
// using Ts::operator()...;
// };
// void Generator::visit(Literal &it) {
// returned = std::visit(
// overloads{
// [](std::string &str) {
// return (llvm::Value *)builder->CreateGlobalString(str);
// },
// [](bool &val) {
// return val ? (llvm::Value *)llvm::ConstantInt::getTrue(*context)
// : (llvm::Value *)llvm::ConstantInt::getFalse(
// *context);
// },
// },
// it.data);
// }

void breadth_function_define(FunctionAST *it, Generator &gen) {
	std::vector<llvm::Type *> params(it->proto->parameters.size());

	for (int i = 0; i < it->proto->parameters.size(); i++) {
		auto param = it->proto->parameters[i];
		param->accept(gen);
		params[i] = gen.returned_type;
	}

	it->proto->retType->accept(gen);
	llvm::FunctionType *ft =
		llvm::FunctionType::get(gen.returned_type, params, false);
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

void breadth_struct_define(StructAST *s, Generator &gen) {
	std::cout << "hello struct" << std::endl;
	auto struct_ = llvm::StructType::create(*gen.context, s->name);
	s->llvm_value = struct_;
}
} // namespace FoxLang::IR

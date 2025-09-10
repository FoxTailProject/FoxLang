#include "type_check.hpp"

namespace FoxLang {
void TypeCheck::visit(BlockAST &it) {
	for (auto c : it.content)
		c->accept(*this);
}

void TypeCheck::visit(BinaryExprAST &it) {
	lit_type = null;
	it.LHS->accept(*this);
	auto left = expr_type;
	auto left_lit = lit_type;

	lit_type = null;
	it.RHS->accept(*this);
	auto right = expr_type;
	auto right_lit = lit_type;
	lit_type = null;

	if (left_lit != null && right_lit != null) {
		compare_lit_types(left_lit, right_lit);
		return;
	}

	if (left_lit != null || right_lit != null) {
		compare_lit_types(left, left_lit, right, right_lit);
		return;
	}

	if (*left != *right) std::cout << "Error: types not equal" << std::endl;
	expr_type = left;
}

void TypeCheck::visit(CallExprAST &it) {}

void TypeCheck::visit(NumberExprAST &it) { lit_type = TypeAST::Type::__uint; }
void TypeCheck::visit(StringLiteralAST &it) {}
void TypeCheck::visit(BoolLiteralAST &it) { lit_type = TypeAST::Type::_bool; }
void TypeCheck::visit(StructLiteralAST &it) {
	// TODO: gotta fix the ptr, then go inside and check types
	lit_type = TypeAST::Type::_struct;
}

void TypeCheck::visit(VariableExprAST &it) {}
void TypeCheck::visit(FileAST &it) {
	for (auto c : it.expressions)
		c->accept(*this);
}

void TypeCheck::visit(ParameterAST &it) {}
void TypeCheck::visit(FunctionAST &it) {}
void TypeCheck::visit(PrototypeAST &it) {}
void TypeCheck::visit(ExprStmt &it) {}
void TypeCheck::visit(ReturnStmt &it) {}
void TypeCheck::visit(IfStmt &it) {}
void TypeCheck::visit(WhileStmt &it) {}
void TypeCheck::visit(VarDecl &it) {}
void TypeCheck::visit(TypeAST &it) {}
void TypeCheck::visit(StructMemberAST &it) {}
void TypeCheck::visit(StructAST &it) {}
} // namespace FoxLang

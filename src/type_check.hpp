#pragma once

#include "ast_pass.hpp"
namespace FoxLang {
class TypeCheck : public ASTVisitor {
public:
	TypeAST *current_type;
	TypeAST *expr_type;
	TypeAST::Type lit_type;
	const static TypeAST::Type null = (TypeAST::Type)0;
	const static TypeAST::Type error = (TypeAST::Type)1;

	virtual void visit(BlockAST &it);
	virtual void visit(BinaryExprAST &it);
	virtual void visit(CallExprAST &it);
	virtual void visit(NumberExprAST &it);
	virtual void visit(StringLiteralAST &it);
	virtual void visit(BoolLiteralAST &it);
	virtual void visit(StructLiteralAST &it);
	virtual void visit(VariableExprAST &it);
	virtual void visit(FileAST &it);
	virtual void visit(ParameterAST &it);
	virtual void visit(FunctionAST &it);
	virtual void visit(PrototypeAST &it);
	virtual void visit(ExprStmt &it);
	virtual void visit(ReturnStmt &it);
	virtual void visit(IfStmt &it);
	virtual void visit(WhileStmt &it);
	virtual void visit(VarDecl &it);
	virtual void visit(TypeAST &it);
	virtual void visit(StructMemberAST &it);
	virtual void visit(StructAST &it);

	inline void compare_lit_types(TypeAST::Type left_lit,
								  TypeAST::Type right_lit) {
		using T = TypeAST::Type;
		if (left_lit == T::_bool && right_lit == T::_bool) lit_type = T::_bool;
		if (left_lit == T::__float && right_lit == T::__float)
			lit_type = T::__float;
		if ((left_lit == T::__int && right_lit == T::__int) ||
			(left_lit == T::__uint && right_lit == T::__int) ||
			(left_lit == T::__int && right_lit == T::__uint))
			lit_type = T::__int;
		if (left_lit == T::__uint && right_lit == T::__uint)
			lit_type = T::__uint;
		if (lit_type == null) {
			lit_type = error;
			std::cout << "Types not equal" << std::endl;
		}
	}

	inline void compare_lit_types(TypeAST &type, TypeAST::Type lit) {
		using T = TypeAST::Type;
		if (type.type == T::_bool && lit == T::_bool) {
			expr_type = &type;
			return;
		}
	}

	inline void compare_lit_types(TypeAST &left, TypeAST::Type left_lit,
								  TypeAST &right, TypeAST::Type right_lit) {
		if (left_lit == null)
			compare_lit_types(left, right_lit);
		else
			compare_lit_types(right, left_lit);
	}
};
} // namespace FoxLang

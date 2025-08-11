#include "name_resolution.hpp"
#include "message.hpp"

namespace FoxLang {
void NameResolution::visit(BlockAST &it) {
	scopes.push_back(Scope());

	for (auto i : it.content)
		i->accept(*this);

	scopes.pop_back();
}

void NameResolution::visit(BinaryExprAST &it) {
	it.LHS->accept(*this);
	it.RHS->accept(*this);
}

void NameResolution::visit(CallExprAST &it) {
	if (function_scope.find(it.Callee) == function_scope.end())
		messages.push_back(
			Message{.message = fmt::format("Undefined function {}", it.Callee),
					.level = Severity::Error,
					.code = "E0200",
					.span = Location{
						.fp = "test.fox",
						.line = 100,
						.column = 3,
						.start = 0,
						.end = it.Callee.length(),
					}});

	it.resolved_name = function_scope[it.Callee];

	for (auto i : it.Args)
		i->accept(*this);
}

void NameResolution::visit(NumberExprAST &) {}

void NameResolution::visit(VariableExprAST &it) {
	Scope *i = &scopes.back();

	while (i >= &scopes.front()) {
		if (i->find(it.name) != i->end()) {
			it.resolved_name = (*i)[it.name];
			return;
		}

		i--;
	}

	if (global_scope.find(it.name) != global_scope.end()) {
		it.resolved_name = scopes.back()[it.name];
		return;
	}

	messages.push_back(
		Message{.message = fmt::format("Undefined variable {}", it.name),
				.level = Severity::Error,
				.code = "E0201",
				.span = Location{
					.fp = "test.fox",
					.line = 100,
					.column = 3,
					.start = 0,
					.end = it.name.length(),
				}});
}

void NameResolution::visit(FileAST &it) {
	// do structs before functions because functions can return/use a struct
	// that has not yet been defined
	for (auto i : it.expressions)
		if (auto sit = dynamic_cast<StructAST *>(i.get())) depth_struct(*sit);

	for (auto i : it.expressions) {
		auto fit = dynamic_cast<FunctionAST *>(i.get());
		if (fit != nullptr) depth_proto(*fit->proto);
	}

	for (auto i : it.expressions)
		i->accept(*this);
}

void NameResolution::visit(ParameterAST &it) {
	scopes.back()[it.name] = &it;
	it.type->accept(*this);
}

void NameResolution::visit(FunctionAST &it) {
	it.proto->accept(*this);
	it.body->accept(*this);
}

void NameResolution::visit(PrototypeAST &it) {
	function_scope[it.name] = &it;
	it.retType->accept(*this);

	scopes.push_back(Scope());
	for (auto i : it.parameters) {
		i->accept(*this);
	}
}

void NameResolution::visit(ExprStmt &it) { it.value->accept(*this); }
void NameResolution::visit(Literal &) {}

void NameResolution::visit(ReturnStmt &it) {
	if (it.value) it.value.value()->accept(*this);
}

void NameResolution::visit(IfStmt &it) {
	it.condition->accept(*this);
	it.block->accept(*this);

	if (it.else_) it.else_.value()->accept(*this);
}

void NameResolution::visit(WhileStmt &it) {
	it.condition->accept(*this);
	it.block->accept(*this);
}

void NameResolution::visit(VarDecl &it) {
	scopes.back()[it.name] = &it;

	it.type->accept(*this);

	if (it.value) it.value.value()->accept(*this);
}

std::ostream &operator<<(std::ostream &out, const TypeAST::Type value) {
	using T = TypeAST::Type;
	return out << [value] {
#define PROCESS_VAL(p)                                                         \
	case (p):                                                                  \
		return #p;
		switch (value) {
			PROCESS_VAL(T::i128);
			PROCESS_VAL(T::u128);
			PROCESS_VAL(T::i64);
			PROCESS_VAL(T::u64);
			PROCESS_VAL(T::i32);
			PROCESS_VAL(T::u32);
			PROCESS_VAL(T::i16);
			PROCESS_VAL(T::u16);
			PROCESS_VAL(T::i8);
			PROCESS_VAL(T::u8);
			PROCESS_VAL(T::f128);
			PROCESS_VAL(T::f64);
			PROCESS_VAL(T::f32);
			PROCESS_VAL(T::f16);
			PROCESS_VAL(T::_bool);
			PROCESS_VAL(T::_struct);
			PROCESS_VAL(T::string);
		}
#undef PROCESS_VAL
	}();
}

void NameResolution::visit(TypeAST &it) {
	std::cout << "Type: " << it.type << std::endl;
	std::cout << "type data: " << it.data << std::endl;
	if (it.type != TypeAST::Type::_struct) return;

	if (global_scope.find(it.data) == global_scope.end())
		messages.push_back(
			Message{.message = fmt::format("Undefined type {}", it.data),
					.level = Severity::Error,
					.code = "E0202",
					.span = Location{
						.fp = "test.fox",
						.line = 100,
						.column = 3,
						.start = 0,
						.end = it.data.length(),
					}});

	it.resolved_name = static_cast<StructAST *>(global_scope[it.data]);
}
void NameResolution::visit(StructAST &it) {
	global_scope[it.name] = &it;

	for (auto child : it.types)
		child->accept(*this);
}

void NameResolution::depth_proto(PrototypeAST &it) {
	function_scope[it.name] = &it;
}

void NameResolution::depth_struct(StructAST &it) {
	global_scope[it.name] = &it;
}
} // namespace FoxLang

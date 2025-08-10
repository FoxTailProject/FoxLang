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
	for (auto i : it.expressions) {
		auto fit = dynamic_cast<FunctionAST *>(i.get());
		if (fit != nullptr) depth_proto(*fit->proto);
	}

	for (auto i : it.expressions)
		i->accept(*this);
}

void NameResolution::visit(ParameterAST &it) { scopes.back()[it.name] = &it; }

void NameResolution::visit(FunctionAST &it) {
	it.proto->accept(*this);
	it.body->accept(*this);
}

void NameResolution::visit(PrototypeAST &it) {
	function_scope[it.name] = &it;

	scopes.push_back(Scope());
	for (auto i : it.parameters)
		i->accept(*this);
}

void NameResolution::depth_proto(PrototypeAST &it) {
	function_scope[it.name] = &it;
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

	if (it.value) it.value.value()->accept(*this);
}

void NameResolution::visit(TypeAST &) {}
void NameResolution::visit(StructAST &it) {}
} // namespace FoxLang

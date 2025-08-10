#include <deque>
#include <fstream>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <string>

#include "../vendor/argparse/argparse.hpp"

#include "ast_nodes.hpp"
#include "ast_parser.hpp"
#include "ir_generator.hpp"
#include "lexer.hpp"

#include "message.hpp"
#include "name_resolution.hpp"

void printTree(const std::string &prefix, const FoxLang::AST *node,
			   bool isLeft);
void printTree(const FoxLang::AST *node);
void handle_messages(std::deque<FoxLang::Message> &messages);

auto main(int argc, char *argv[]) -> int {
	argparse::ArgumentParser program("fox", "0.0.1 epsilon");

	argparse::ArgumentParser compile_command("compile");
	compile_command.add_argument("--print-ast").flag();
	compile_command.add_argument("-o", "--output").nargs(1);
	compile_command.add_argument("files").required().nargs(1);

	program.add_subparser(compile_command);

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}

	auto file_name = compile_command.get<std::string>("files");
	std::ifstream file(file_name);

	if (!file.is_open()) {
		std::cerr << "Could not open file `" << file_name << "`" << std::endl;
		std::exit(1);
	}

	std::deque<FoxLang::Message> messages;

	std::string contents((std::istreambuf_iterator<char>(file)),
						 (std::istreambuf_iterator<char>()));

	FoxLang::Lexer lexer(&contents, file_name, messages);
	std::vector<FoxLang::Token> *tokens = lexer.Lex();

	FoxLang::Parser ast(tokens, messages);
	auto tree = ast.parse();

	FoxLang::NameResolution nr(messages);
	tree->accept(nr);

	bool erred = false;
	for (auto i : messages) {
		if (i.level == FoxLang::Severity::Error) {
			erred = true;
			break;
		}
	}

	handle_messages(messages);
	if (erred) return 1;

	if (compile_command["print-ast"] == true) printTree(tree);

	FoxLang::IR::Generator ir;
	tree->accept(ir);

	return 0;
}

void handle_messages(std::deque<FoxLang::Message> &messages) {
	while (!messages.empty()) {
		messages.front().print();
		messages.pop_front();
	}
}

void printTree(const std::string &prefix, const FoxLang::AST *node,
			   bool isLast) {
	if (node != nullptr) {
		std::cout << prefix;

		std::cout << (isLast ? "└──" : "├──");

		// print the value of the node
		std::cout << node->printName() << std::endl;

		auto children = node->getChildren();
		if (children.empty()) return;

		// enter the next tree level - left and right branch
		for (size_t i = 0; i < children.size() - 1; i++) {
			printTree(prefix + (!isLast ? "│   " : "    "), children[i], false);
		}
		printTree(prefix + (!isLast ? "│   " : "    "), children.back(), true);
	}
}

void printTree(const FoxLang::AST *node) { printTree("", node, false); }

#include <fstream>
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <string>

#include "../vendor/argparse/argparse.hpp"

#include "ast/nodes.h"
#include "ast/parser.h"
#include "lexer/tokenizer.h"

void printTree(const std::string &prefix, const FoxLang::AST *node,
			   bool isLeft);
void printTree(const FoxLang::AST *node);

auto main(int argc, char *argv[]) -> int {
	argparse::ArgumentParser program("fox", "0.0.1 epsilon");

	argparse::ArgumentParser compile_command("compile");
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

	std::string contents((std::istreambuf_iterator<char>(file)),
						 (std::istreambuf_iterator<char>()));

	// std::string line;
	// while (std::getline(file, line)) {
	//   std::cout << line << std::endl;
	// }
	FoxLang::Tokenizer tokenizer(&contents);
	std::vector<FoxLang::Token> *tokens = tokenizer.Tokenize();

	// FoxLang::ASTGenerator astGenerator(tokens);
	// auto fileTree = astGenerator.GenerateFileTree();

	// FoxLang::AST::llvm_module = std::make_unique<llvm::Module>();
	FoxLang::Parser ast(tokens);
	auto tree = ast.parse();
	printTree(tree);

	return 0;
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

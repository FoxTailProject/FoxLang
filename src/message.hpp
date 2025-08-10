#pragma once

#include <fmt/color.h>
#include <fmt/printf.h>
#include <string>

namespace FoxLang {
enum class Severity;
struct Location {
	std::string fp;
	unsigned long line, column;
	unsigned long start, end;
};

struct Message {
	std::string message;
	Severity level;
	std::string code;
	Location span;

	void print();
};

enum class Severity { Error, Warning };
} // namespace FoxLang

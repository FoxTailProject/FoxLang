#include "message.hpp"
#include <cctype>

namespace FoxLang {
auto strip_whitespace(std::string str) {
	auto start = str.begin();
	int count = 0;
	while (std::isspace(*start)) {
		start++;
		count++;
	}

	auto end = str.end();
	while (std::isspace(*end))
		end--;
	struct {
		std::string str;
		int count;
	} ret = {.str = std::string(start, end), .count = count};
	return ret;
}

void Message::print() {
	if (level == Severity::Error)
		fmt::print(fmt::emphasis::bold | fg(fmt::color::crimson), "error");
	else
		fmt::print(fmt::emphasis::bold | fg(fmt::color::light_yellow),
				   "warning");
	fmt::print(fmt::emphasis::bold, ": {}\n", message);
	fmt::print(fg(fmt::color::light_green), " --> ");
	fmt::print("{}:{}:{}\n", span.fp, span.line, span.column);

	int digits = ceil(log10(span.line));

	fmt::print(fg(fmt::color::light_sky_blue), "{:>{}} |\n", "", digits);
	fmt::print(fg(fmt::color::light_sky_blue), "{} |  ", span.line);

	std::ifstream file(span.fp);
	std::string line;
	int l = 0;
	while (l < span.line) {
		std::getline(file, line);
		l++;
	}

	auto line_stripped = strip_whitespace(line);
	fmt::print("{}\n", line_stripped.str);
	fmt::print(fg(fmt::color::light_sky_blue), "{:>{}} | {:>{}}", "", digits,
			   "", span.column - line_stripped.count);
	fmt::print(fg(fmt::color::crimson), "{:^>{}}\n\n", "", span.len);
}
} // namespace FoxLang

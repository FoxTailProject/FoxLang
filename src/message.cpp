#include "message.hpp"
#include <cctype>
#include <cmath>
#include <fstream>

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
		fmt::print(fmt::emphasis::bold | fg(fmt::color::crimson), "error {}",
				   code);
	else
		fmt::print(fmt::emphasis::bold | fg(fmt::color::light_yellow),
				   "warning {}", code);
	fmt::print(fmt::emphasis::bold, ": {}\n", message);
	fmt::print(fg(fmt::color::light_green), " --> ");
	fmt::print("{}:{}:{}\n", span.fp, span.line, span.column);

	int digits = ceil(log10(span.line));

	fmt::print(fg(fmt::color::light_sky_blue), "{} |\n",
			   std::string(digits + 1, ' '));
	fmt::print(fg(fmt::color::light_sky_blue), "{} |  ", span.line);

	unsigned long len = span.end - span.start;
	std::ifstream file(span.fp);
	file.seekg(span.start);
	// std::string line;
	char line[len + 1];
	file.read(line, len);

	fmt::print("{}\n", std::string(line));
	fmt::print(fg(fmt::color::light_sky_blue), "{} | {}",
			   std::string(digits + 1, ' '),
			   std::string(span.column - len, ' '));
	fmt::print(fg(fmt::color::crimson), "{:^>{}}\n\n", "", len);
}
} // namespace FoxLang

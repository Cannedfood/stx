#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace stx {

class csv_parser {
	char sep;

	std::istream&                 data;
	std::string                   line;
	std::vector<std::string_view> entries;
public:
	csv_parser(std::istream& data, char sep = ',') : data(data), sep(sep) {}

	bool next() {
		if(!std::getline(data, line)) return false;

		const char* entryStart = line.c_str();

		bool next = true;
		while(next) {
			if(*entryStart == '"')
				next = parseEscaped(entryStart);
			else
				next = parseSimple();
		}
	}
};

} // namespace stx
#include "parsing.hpp"

#include <algorithm>
#include <iostream>

namespace stx::parsing {

cursor_location::cursor_location(unsigned line, unsigned column) :
	line(line),
	column(column)
{}
cursor_location::cursor_location(const char* start, const char* location) :
	cursor_location(1u, 1u)
{
	const char* line_start = start;
	const char* line_end;
	// Get line, column and line_start
	while(start < location) {
		switch(*start) {
			case '\r':
				if(start[1] == '\n')
					start++;
				[[fallthrough]];
			case '\n':
				++line;
				column = 1;
				line_start = start + 1;
				break;
			default:
				++column;
				break;
		}
		++start;
	}
	// Get line_end
	line_end = line_start;
	while(*line_end && *line_end != '\n' && *line_end != '\r')
		++line_end;

	line_content = {line_start, size_t(line_end - line_start)};
}

namespace errors {

static
std::string _marker_indention(cursor_location const& loc) {
	auto marker_indention = std::string(
		loc.line_content.data(),
		loc.column - 1
		);

	std::replace_if(
		marker_indention.begin(), marker_indention.end(),
		[](char c) { return c != ' ' && c != '\t'; }, ' '
	);

	return marker_indention;
}

void error::diagnose(const char* srcfile, const char* source, std::ostream& stream) {
	if(source) {
		auto location1 = cursor_location(source, this->location());
		stream
		<< srcfile << ":" << location1.line << ":" << location1.column
		<< ": Parsing error: " << m_message << '\n'
		<< '\t' << std::string_view(location1.line_content) << '\n'
		<< '\t' << _marker_indention(location1) << "^ here"
		<< std::endl;

		if(m_location2.size()) {
			auto location2 = cursor_location(source, m_location2.data());
			stream
			<< "\t While parsing construct: \n"
			<< '\t' << location2.line_content << '\n'
			<< '\t' << _marker_indention(location2) << std::string('~', std::max(size_t(1), m_location2.size()) -  1)
			<< std::endl;
		}
	}
	else {
		stream << "Parsing error: " << m_message << std::endl;
	}
}

void error::diagnose(const char* srcfile, const char* source) {
	diagnose(srcfile, source, std::cerr);
}

} // namespace errors

} // namespace stx::parsing
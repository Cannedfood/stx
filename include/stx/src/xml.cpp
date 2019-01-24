#include "../xml.hpp"

#include "../platform.hpp"

#include <fstream>

#include <cctype>
#include <cstring>
#include <charconv>
#include <algorithm>

#include <iostream>

#include <array>

namespace stx {
namespace xml {

template<class Pred> constexpr
std::array<bool, 256> _character_lookup(Pred pred) {
	std::array<bool, 256> values { false };
	for(unsigned char c = 0; c < 255; c++) {
		values[c] = pred((char) c);
	}
	return values;
}

static
void next_token(const char*& s) {
	// TODO: conformance
	while(*s != '\0' && *s <= ' ') {
		s++;
	}
}

static
bool is_name_character(char c) {
	constexpr auto lookup = _character_lookup([](char c) {
		bool is_ws_or_control = c <= ' ';
		bool is_delete        = c == 127;
		bool is_special_char  = (c == '>') || (c == '/') || (c == '=');

		return !is_ws_or_control && !is_delete && !is_special_char;
	});

	return lookup[c];
}

static
std::string_view trim_whitespace(std::string_view s) {
	while(s.size() && std::iswspace(s.front())) s.remove_prefix(1);
	while(s.size() && std::iswspace(s.back())) s.remove_suffix(1);
	return s;
}

static
std::string_view parse_name(const char*& s) {
	// TODO: conformance
	const char* beg = s;
	while(is_name_character(*s)) {
		s++;
	}

	if(STX_UNLIKELY(beg == s)) {
		throw errors::parsing_error("Expected valid name", s);
	}

	return {beg, size_t(s - beg)};
}

static
std::string_view parse_literal(const char*& s) {
	if(*s == '\'') {
		s++;
		const char* start = s;
		while(*s && *s != '\'') s++;
		if(!*s) {
			throw errors::parsing_error(
				"Expected closing single quote ' character",
				s, { start, 1 }
			);
		}
		auto result = std::string_view(start, size_t(s - start));
		s++;
		return result;
	}
	else if(*s == '"') {
		s++;
		const char* start = s;
		while(*s && *s != '"') s++;
		if(!*s) {
			throw errors::parsing_error(
				"Expected closing double quote \" character",
				s, { start, 1 }
			);
		}
		auto result = std::string_view(start, size_t(s - start));
		s++;
		return result;
	}
	else {
		throw errors::parsing_error("Expected literal (e.g. 'literal' or \"literal\")", s);
	}
}

attribute* attribute::next(std::string_view name) noexcept {
	attribute* n = this;
	while((n = n->next())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}
attribute* attribute::prev(std::string_view name) noexcept {
	attribute* n = this;
	while((n = n->prev())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}
attribute& attribute::req_next(std::string_view name) {
	attribute* result = next(name);
	if(!result) {
		throw errors::attribute_not_found(
			"Expected attribute '" + std::string(name) + "' after that one",
			m_value.data()
		);
	}
	return *result;
}
attribute& attribute::req_prev(std::string_view name) {
	attribute* result = prev(name);
	if(!result) {
		throw errors::attribute_not_found(
			"Expected attribute '" + std::string(name) + "' before this one",
			m_value.data()
		);
	}
	return *result;
}

const char* attribute::parse(arena_allocator& alloc, const char* s) {
	m_name = parse_name(s);
	if(*s != '=') {
		m_value = std::string_view();
		return s;
	}
	s++;
	m_value = parse_literal(s);
	return s;
}

template<>
bool attribute::value<bool>() const {
	return m_value == "1" || m_value == "true";
}
template<>
int attribute::value<int>() const {
	int result;
	std::from_chars_result error = std::from_chars(m_value.data(), m_value.data() + m_value.length(), result);
	if(error.ec != std::errc()) {
		throw errors::parsing_error("Expected a valid number value", m_value.data(), m_value);
	}
	return result;
}
template<>
float attribute::value<float>() const {
	return std::stof(std::string(m_value));
	/*
	float result;
	std::from_chars_result error = std::from_chars(m_value.data(), m_value.data() + m_value.length(), result);
	if(error.ec != std::errc()) {
		throw errors::parsing_error("Expected a valid number value", m_value.data(), m_value);
	}
	return result;
	*/
}

bool node::name_in(std::initializer_list<std::string_view> const& names) const noexcept {
	for(auto& name : names) {
		if(this->name() == name) return true;
	}
	return false;
}

node* node::first(std::string_view name) noexcept {
	node* n = this;
	do {
		if(n->name() == name)
			return n;
	} while((n = n->next()));
	return nullptr;
}
node* node::next(std::string_view name) noexcept {
	if(!next()) return nullptr;
	return next()->first(name);
}
node* node::prev(std::string_view name) noexcept {
	node* n = this;
	while((n = n->prev())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}

node* node::first_of(std::initializer_list<std::string_view> const& names) noexcept {
	node* n = this;
	do {
		if(n->name_in(names)) return n;
	} while((n = n->next()));
	return nullptr;
}
node* node::next_of(std::initializer_list<std::string_view> const& names) noexcept {
	if(!next()) return nullptr;
	return next()->first_of(names);
}

node* node::child(std::initializer_list<std::string_view> const& names) noexcept {
	if(!children()) return nullptr;
	return children()->first_of(names);
}
node* node::child(std::string_view name) noexcept {
	if(!children()) return nullptr;
	return children()->first(name);
}
node* node::child(node::node_type type) noexcept {
	node* n = children();
	while(n) {
		if(n->type() == type)
			return n;
		n = n->next();
	}
	return nullptr;
}

node& node::req_child(node::node_type type) {
	auto* result = child(type);
	if(!result) {
		throw errors::node_not_found(
			"Node doesn't have required child of type " + std::to_string(type) + "",
			m_value.data()
		);
	}
	return *result;
}

node& node::req_child(std::string_view name) {
	auto* result = child(name);
	if(!result) {
		throw errors::node_not_found(
			"Node doesn't have required child '" + std::string(name) + "'",
			m_value.data()
		);
	}
	return *result;
}

attribute* node::attrib(std::string_view name) noexcept {
	attribute* n = attributes();
	while(n) {
		if(n->name() == name)
			return n;
		n = n->next();
	}
	return nullptr;
}
attribute& node::req_attrib(std::string_view name) {
	auto* result = attrib(name);
	if(!result) {
		throw errors::attribute_not_found(
			"Node doesn't have required attribute '" + std::string(name) + "'",
			m_value.data()
		);
	}
	return *result;
}

const char* node::parse_regular(arena_allocator& alloc, const char* s) {
	if(*s != '<')
		throw errors::parsing_error("Expected opening less-than sign <", s);
	s++;

	m_value = parse_name(s);
	m_type = regular;

	next_token(s);
	s = parse_attributes(alloc, s);
	if(*s == '/') {
		s++;
		if(STX_UNLIKELY(*s != '>')) {
			throw errors::parsing_error("Expected closing greater-than sign >", s, m_value);
		}
		return ++s; // No body
	}
	if(STX_UNLIKELY(*s != '>')) {
		throw errors::parsing_error("Expected closing greater-than sign >", s, m_value);
	}
	s++;

	// Parse body
	next_token(s);
	s = parse_children(alloc, s);

	s += 2;

	if(auto closing_name = parse_name(s); closing_name != m_value)
		errors::parsing_error("Closing tag doesn't match opening tag", closing_name.data(), m_value.data());

	next_token(s);
	if(*s != '>')
		throw errors::parsing_error("Expected closing greater-than sign >", s);

	return ++s;
}
const char* node::parse_doctype(arena_allocator& alloc, const char* s) {
	m_type = doctype;
	while(*s && *s != '>') // HACK, actually parse the thing
		s++;
	if(*s != '>')
		throw errors::parsing_error("Expected closing greater-than sign", s);
	s++;
	return s;
}
const char* node::parse_comment(arena_allocator& alloc, const char* s) {
	m_type = comment;
	s += 4;
	const char* start = s;
	while(*s) {
		if(s[0] == '-' && s[1] == '-' && s[2] == '>') {
			m_value = trim_whitespace({start, size_t(s - start)});
			return s + 3;
		}
		s++;
	}
	throw errors::parsing_error("Expected closing greater-than sign", s);
}
const char* node::parse_content(arena_allocator& alloc, const char* s) {
	while(*s <= ' ') s++; // Skipws
	m_type = content;
	const char* start = s;
	while(*s && (s[0] != '<' || s[1] <= ' ')) s++;
	m_value = std::string_view(start, std::max(start, s - 1) - start);
	while(!m_value.empty() && m_value.back() <= ' ') m_value.remove_suffix(1);
	return s;
}
const char* node::parse_attributes(arena_allocator& alloc, const char* s) {
	while(*s != '/' && *s != '>') {
		attribute* attrib = alloc.create<attribute>();
		s = attrib->parse(alloc, s);

		attrib->push_tail(&m_attributes);
		next_token(s);
	}

	// Shift attributes into the correct order
	if(m_attributes) {
		while(auto* p = m_attributes->prev())
			m_attributes = p;
	}
	return s;
}
const char* node::parse_children(arena_allocator& alloc, const char* s) {
	next_token(s);
	while(*s && !(s[0] == '<' && s[1] == '/')) {
		node* n = alloc.create<node>();
		s = n->parse_node(alloc, s);
		n->m_parent = this;
		n->push_tail(&m_children);
		next_token(s);
	}
	if(m_children) {
		while(auto* p = m_children->prev())
			m_children = p;
	}
	return s;
}

const char* node::parse_node(arena_allocator& alloc, const char* s) {
	next_token(s);

	if(*s != '<')
		return parse_content(alloc, s);

	if(STX_UNLIKELY(s[1] == '!')) {
		if(STX_LIKELY(s[2] == '-' && s[3] == '-')) {
			return parse_comment(alloc, s);
		}
		else if(!strcmp(s + 2, "DOCTYPE")) {
			return parse_doctype(alloc, s);
		}
		else {
			throw errors::parsing_error(
				"Expected "
				"comment (e.g. <!-- comment -->) or "
				"DOCTYPE declaration (e.g. <!DOCTYPE html>)"
			);
		}
	}
	return parse_regular(alloc, s);
}

const char* node::parse_document(arena_allocator& alloc, const char* s) {
	next_token(s);
	if(*s != '<') {
		throw errors::parsing_error(
			"Expected entity (e.g. <entity/>), "
			"comment (e.g <!-- Comment -->) or "
			"doctype declaration <!DOCTYPE>", s);
	}

	parse_children(alloc, s);

	return s;
}

void node::print(std::ostream& stream, unsigned indent) {
	switch(type()) {
		case node_type::comment: {
			stream
			<< std::string(indent * 2, ' ')
			<< "<!--" << m_value << "-->" << std::endl;
		} return;
		case node_type::content: {
			stream
			<< std::string(indent * 2, ' ')
			<< m_value << std::endl;
		} return;
		default: {
			stream
			<< std::string(indent * 2, ' ')
			<< '<' << m_value;
			for(auto* atb = attributes(); atb; atb = atb->next()) {
				stream
				<< '\n' << std::string(indent * 2 + 2, ' ')
				<< atb->name() << "='" << atb->value() << '\'';
			}
			if(children()) {
				stream << ">\n";
				for(auto* child = children(); child; child = child->next())
					child->print(stream, indent + 1);
				stream
				<< std::string(indent * 2, ' ')
				<< "</" << m_value << ">\n";
			}
			else {
				stream << "/>\n";
			}
		} return;
	}
}

std::string load_document(std::string_view path) {
	std::string result;

	// Open file
	auto file = std::ifstream(std::string(path));
	if(!file) throw std::runtime_error("Couldn't open file");

	// Get size of file and resize string
	file.seekg(0, std::ios::end);
	result.resize(file.tellg());
	file.seekg(0, std::ios::beg);

	// Read data into string
	file.read(result.data(), result.size());
	if(file.gcount() != static_cast<std::streamoff>(result.size()))
		throw std::runtime_error("Couldn't read full file");

	return result;
}

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

} // namespace xml
} // namespace stx

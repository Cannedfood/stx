#include "xml.hpp"

#include <fstream>

#include <cctype>
#include <cstring>
#include <charconv>
#include <algorithm>

#include <iostream>

#include <array>

namespace stx::xml {

static
void next_token(const char*& s) {
	// TODO: conformance
	while(*s != '\0' && *s <= ' ') {
		s++;
	}
}

static
bool is_name_character(char c) {
	constexpr auto lookup = parsing::make_lookup([](char c) {
		bool is_ws_or_control = c <= ' ';
		bool is_delete        = c == 127;
		bool is_special_char  = (c == '>') || (c == '/') || (c == '=');

		return !is_ws_or_control && !is_delete && !is_special_char;
	});

	return lookup[c];
}

static
std::string_view trim_whitespace(std::string_view s) {
	while(s.size() && std::isspace(s.front())) s.remove_prefix(1);
	while(s.size() && std::isspace(s.back())) s.remove_suffix(1);
	return s;
}

static
std::string_view parse_name(const char*& s) {
	// TODO: conformance
	const char* beg = s;
	while(is_name_character(*s)) {
		s++;
	}

	if(beg == s) {
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
unsigned attribute::value<unsigned>() const {
	unsigned result;
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
template<>
std::string attribute::value<std::string>() const {
	return std::string(this->value());
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
		if(n->type() == regular && n->name() == name)
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
		if(n->type() == regular && n->name() == name)
			return n;
	}
	return nullptr;
}

node* node::first_of(std::initializer_list<std::string_view> const& names) noexcept {
	node* n = this;
	do {
		if(n->type() == regular && n->name_in(names)) return n;
	} while((n = n->next()));
	return nullptr;
}
node* node::next_of(std::initializer_list<std::string_view> const& names) noexcept {
	if(!next()) return nullptr;
	return next()->first_of(names);
}

node* node::next(node::node_type type) noexcept {
	node* n = this;
	while((n = n->next())) {
		if(n->type() == type) return n;
	}
	return nullptr;
}
node* node::prev(node::node_type type) noexcept {
	node* n = this;
	while((n = n->prev())) {
		if(n->type() == type) return n;
	}
	return nullptr;
}
node& node::req_next(node::node_type type) {
	node* result = next(type);
	if(!result) {
		throw errors::node_not_found(
			"Node doesn't have required child of type " + std::to_string(type) + "",
			m_value.data()
		);
	}
	return *result;
}
node& node::req_prev(node::node_type type) {
	node* result = prev(type);
	if(!result) {
		throw errors::node_not_found(
			"Node doesn't have required child of type " + std::to_string(type) + "",
			m_value.data()
		);
	}
	return *result;
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
	s = parse_attributes(alloc, s, '/');
	if(*s == '/') {
		s++;
		if(*s != '>') {
			throw errors::parsing_error("Expected closing greater-than sign >", s, m_value);
		}
		return ++s; // No body
	}
	if(*s != '>') {
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
const char* node::parse_processing_instruction(arena_allocator& alloc, const char* s) {
	m_type = processing_instruction;

	if(s[0] != '<' || s[1] != '?') {
		throw parsing::errors::parsing_error("Expected processing instruction starting with <?", s);
	}

	s += 2;

	next_token(s);
	this->m_value = parse_name(s);

	next_token(s);
	parse_attributes(alloc, s, '?');

	while(!(s[0] == '?' && s[1] == '>')) {
		if(s[1] == '\0') {
			throw parsing::errors::parsing_error("Expected '?>' matching '<?' (End of processing instruction)", s);
		}
		s++;
	}

	return s + 2;
}
const char* node::parse_content(arena_allocator& alloc, const char* s) {
	while(*s <= ' ') s++; // Skipws
	m_type = content;
	const char* start = s;
	while(*s && (s[0] != '<' || s[1] <= ' ')) s++;
	m_value = std::string_view(start, std::max(start, s) - start);
	while(!m_value.empty() && m_value.back() <= ' ') m_value.remove_suffix(1);
	return s;
}
const char* node::parse_attributes(arena_allocator& alloc, const char* s, const char endChar) {
	while((*s != endChar) && *s != '>') {
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

	if(s[1] == '!') {
		if(s[2] == '-' && s[3] == '-') {
			return parse_comment(alloc, s);
		}
		else if(
			0 == memcmp(s + 2, "DOCTYPE", 7) ||
			0 == memcmp(s + 2, "ELEMENT", 7) ||
			0 == memcmp(s + 2, "ATTLIST", 7) ||
			0 == memcmp(s + 2, "ENTITY",  6)
		) {
			return parse_doctype(alloc, s);
		}
		else {
			throw errors::parsing_error(
				"Expected "
				"comment (e.g. <!-- comment -->) or "
				"DOCTYPE declaration (e.g. <!DOCTYPE html>)"
				"after <!"
			);
		}
	}

	if(s[1] == '?') {
		return parse_processing_instruction(alloc, s);
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

static std::string _load_file(std::string_view path) {
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


document document::parse(const char* text) {
	document result;
	result.parse_document(result.allocator, text);
	return result;
}
document document::load(const char* path) {
	return document::parse(_load_file(path).c_str());
}

} // namespace stx::xml

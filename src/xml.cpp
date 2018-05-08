#include "../xml.hpp"

#include "../platform.hpp"

#include <fstream>

#include <cctype>
#include <cstring>

namespace stx {
namespace xml {

static
void next_token(const char*& s) {
	// TODO: conformance
	while(*s != '\0' && *s <= ' ')
	{
		s++;
	}
}

static
std::string_view parse_name(const char*& s) {
	// TODO: conformance
	const char* beg = s;
	while(std::isalnum(*s)) {
		s++;
	}

	if(STX_UNLIKELY(beg == s)) {
		throw error("Expected valid name", s);
	}

	return {beg, size_t(s - beg)};
}

static
std::string_view parse_literal(const char*& s) {
	const char* start = s;
	if(*s == '\'') {
		s++;
		while(*s && *s != '\'') s++;
		if(!*s) {
			throw error(
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
		while(*s && *s != '"') s++;
		if(!*s) {
			throw error(
				"Expected closing double quote \" character",
				s, { start, 1 }
			);
		}
		auto result = std::string_view(start, size_t(s - start));
		s++;
		return result;
	}
	else {
		throw error("Expected literal (e.g. 'literal' or \"literal\")", s);
	}
}

attribute* attribute::next(std::string_view const& name) noexcept {
	attribute* n = this;
	while((n = n->next())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}
attribute* attribute::prev(std::string_view const& name) noexcept {
	attribute* n = this;
	while((n = n->prev())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}

const char* attribute::parse(arena_allocator* alloc, const char* s) {
	m_name = parse_name(s);
	if(*s != '=')
		throw error("Expected equals sign =", s, m_name);
	s++;
	m_value = parse_literal(s);
	return s;
}

node* node::next(std::string_view const& name) noexcept {
	node* n = this;
	while((n = n->next())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}
node* node::prev(std::string_view const& name) noexcept {
	node* n = this;
	while((n = n->prev())) {
		if(n->name() == name)
			return n;
	}
	return nullptr;
}

node* node::child(std::string_view const& name) noexcept {
	node* n = children();
	while(n) {
		if(n->name() == name)
			return n;
		n = n->next();
	}
	return nullptr;
}
attribute* node::attrib(std::string_view const& name) noexcept {
	attribute* n = attributes();
	while(n) {
		if(n->name() == name)
			return n;
		n = n->next();
	}
	return nullptr;
}

const char* node::parse_regular(arena_allocator* alloc, const char* s) {
	if(*s != '<')
		throw error("Expected opening less-than sign <", s);
	s++;

	m_name = parse_name(s);
	m_type = regular_node;

	next_token(s);
	s = parse_attributes(alloc, s);
	if(*s == '/') {
		s++;
		if(STX_UNLIKELY(*s != '>')) {
			throw error("Expected closing greater-than sign >", s, m_name);
		}
		return ++s; // No body
	}
	if(STX_UNLIKELY(*s != '>')) {
		throw error("Expected closing greater-than sign >", s, m_name);
	}
	s++;

	// Parse body
	next_token(s);
	s = parse_children(alloc, s);

	s += 2;

	if(auto closing_name = parse_name(s); closing_name != m_name)
		error("Closing tag doesn't match opening tag", closing_name.data(), m_name.data());

	next_token(s);
	if(*s != '>')
		throw error("Expected closing greater-than sign >", s);

	return ++s;
}
const char* node::parse_doctype(arena_allocator* alloc, const char* s) {
	m_type = doctype_node;
	while(*s && *s != '>') // HACK, actually parse the thing
		s++;
	if(*s != '>')
		throw error("Expected closing greater-than sign", s);
	s++;
	return s;
}
const char* node::parse_comment(arena_allocator* alloc, const char* s) {
	m_type = comment_node;
	while(*s && *s != '>') // HACK, actually parse the thing
		s++;
	if(*s != '>')
		throw error("Expected closing greater-than sign", s);
	s++;
	return s;
}
const char* node::parse_content(arena_allocator* alloc, const char* s) {
	m_type = content_node;
	printf("Content node: %s\n", s);
	while(*s && *s != '<') // HACK, actually parse the thing
		s++;
	return s;
}
const char* node::parse_attributes(arena_allocator* alloc, const char* s) {
	do {
		if(*s == '/') {
			return s; // No body, we can return
		}
		else if(*s == '>') {
			break;
		}
		else {
			attribute* attrib = alloc->create<attribute>();
			s = attrib->parse(alloc, s);

			if(m_attributes)
				m_attributes->insert_next(attrib);
			m_attributes = attrib;
		}
		next_token(s);
	} while(true);

	// Shift attributes into the correct order
	if(m_attributes) {
		while(auto* prev = m_attributes->prev())
			m_attributes = prev;
	}
	return s;
}
const char* node::parse_children(arena_allocator* alloc, const char* s) {
	while(*s && !(s[0] == '<' && s[1] == '/')) {
		node* n = alloc->create<node>();
		printf("%p\n", n);
		s = n->parse(alloc, s);
		next_token(s);
		n->m_parent = this;
		if(m_children)
			m_children->insert_next(n);
		m_children = n;
	}
	if(m_children) {
		while(auto* p = m_children->prev())
			m_children = p;
	}
	return s;
}

const char* node::parse(arena_allocator* alloc, const char* s) {
	next_token(s);

	if(*s != '<')
		return parse_content(alloc, s);

	if(STX_UNLIKELY(s[1] == '!')) {
		s++;
		if(!strcmp(s, "DOCTYPE"))
			return parse_doctype(alloc, s);
		else
			return parse_comment(alloc, s);
	}
	return parse_regular(alloc, s);
}

parser::parser() {}

node* parser::parse(const char* cstr) {
	next_token(cstr);
	if(*cstr != '<')
		throw error("Expected entity (e.g. <entity/>)", cstr);
	node* n = create<node>();
	n->parse(this, cstr);
	return n;
}

node* parser::load(const char* path) {
	auto file = std::ifstream(path);
	if(!file) throw std::runtime_error("Couldn't open file");

	// Get size of file
	std::streamsize size;
	file.seekg(0, std::ios::end);
	size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Allocate buffer
	char* data = alloc_string(size);

	// Read data
	file.read(data, size);
	if(file.gcount() != size)
		throw std::runtime_error("Couldn't read full file");

	// Parse
	return parse(data);
}

cursor_location::cursor_location(unsigned line, unsigned column) :
	line(line),
	column(column)
{}
cursor_location::cursor_location(const char* start, const char* location) :
	cursor_location(0u, 0u)
{
	while(start < location) {
		switch(*start) {
			case '\r':
				if(start[1] == '\n')
					start++;
					[[fallthrough]];
			case '\n':
				++line;
				column = 0;
				break;
		}
		++start;
	}
}

} // namespace xml
} // namespace stx

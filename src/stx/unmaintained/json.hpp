#pragma once

#include <string_view>

#include "allocator.hpp"
#include "parsing.hpp"

namespace stx::json {

namespace errors = ::stx::parsing::errors;

struct document_handler {
	virtual void beginObject()               = 0;
	virtual void key(std::string_view value) = 0;
	virtual void endObject()                 = 0;

	virtual void beginArray() = 0;
	virtual void endArray()   = 0;

	virtual void number(std::string_view value) = 0;
	virtual void string(std::string_view value) = 0;
};

const char* parse(document_handler& h, const char* text);

} // namespace stx::json

// =============================================================
// == Inline Implementation =============================================
// =============================================================

namespace stx::json {

namespace detail {

static const char* parseObject(document_handler& h, const char* text);
static const char* parseArray (document_handler& h, const char* text);
static const char* parseString(document_handler& h, const char* text);
static const char* parseNumber(document_handler& h, const char* text);
static const char* parseAny   (document_handler& h, const char* text);

static
const char* skipws(const char* s) {
	while(s[0] && s[0] <= ' ') s++;
	return s;
}

static
const char* parseObject(document_handler& h, const char* text) {
	h.beginObject();

	while(true) {
		text = skipws(text + 1);
		if(text[0] == '}') break;
		if(text[0] != '"') throw parsing::errors::error("Expected a \" (key) or } (end of object). Note that keys have to be \"quoted\"", text);

		text = text + 1;

		size_t len = 0;
		while(text[len] && text[len] != '"') len++;
		if(text[len] != '"') throw parsing::errors::error("Expected key to end with an \". Note that keys have to be \"quoted\"", text);
		h.key(std::string_view(text, len));
		text = text + len + 1;

		text = skipws(text);
		if(text[0] != ':') throw parsing::errors::error("Expected : after key (e.g. \"size\": \"humongous\"");
		text = skipws(text + 1);
		text = parseAny(h, text);
		text = skipws(text);

		if(text[0] == ',') continue;
		if(text[0] == '}') break;
	}

	h.endObject();
	return text + 1;
}

static
const char* parseArray(document_handler& h, const char* text) {
	h.beginArray();

	while(true) {
		text = skipws(text + 1);
		text = parseAny(h, text);
		text = skipws(text);
		if(text[0] == ',') continue;
		if(text[0] == ']') break;
		throw parsing::errors::error("Expected , (for another element) or ] (end of array) at this point in the array", text);
	}

	h.endArray();
	return text + 1;
}

static
const char* parseString(document_handler& h, const char* text) {
	const char* s = text + 1;
	for(size_t i = 0; s[i]; i++) {
		if(s[i] == '"') {
			h.string(std::string_view(s, i));
			return s + i + 1;
		}
	}
	throw parsing::errors::error("Expected string to close with a \"", text, std::string_view(text, 1));
}

static
const char* parseNumber(document_handler& h, const char* text) {
	constexpr static auto isNumchar = parsing::make_lookup([](char c) {
		if(c >= '0' && c <= '9') return true;
		if(c == '.') return true; // Decimal point
		if(c == 'e') return true; // For scientific notation (e.g. 1e7 for 10,000,000)
		return false;
	});

	size_t i = 0;
	while(isNumchar[text[i]]) i++;
	h.number(std::string_view(text, i));
	return text + i;
}

static
const char* parseAny(document_handler& h, const char* text) {
	switch (*text)
	{
		case '\0':
			return text;
		case '{':
			return parseObject(h, text);
		case '[':
			return parseArray(h, text);
		case '"':
			return parseString(h, text);
		default:
			return parseNumber(h, text);
	}
}

} // namespace detail

const char* parse(document_handler& h, const char* text) {
	try {
		return detail::parseAny(h, detail::skipws(text));
	}
	catch(parsing::errors::error& e) {
		e.diagnose("", text);
		throw;
	}
}

} // namespace stx::json

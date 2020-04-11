#ifndef STX_ENVIRONMENT_HPP_INCLUDED
#define STX_ENVIRONMENT_HPP_INCLUDED

#pragma once

#include <string>
#include <algorithm>
#include <stdexcept> // std::runtime_error
#include <cstdlib> // set/get_env
#include <cstring> // set/get_env

#include <cassert>

namespace stx {

class environment {
	virtual const char* get(const char* name) {
		return getenv(name);
	}

	virtual void set(const char* name, const char* value, bool allowOverwrite) {
		setenv(name, value, allowOverwrite);
	}

	virtual void unset(const char* name) {
		unsetenv(name);
	}
};

namespace env {

inline
const char* get(const char* name) {
	return getenv(name);
}

inline
std::string gets(const char* name) {
	if(const char* val = getenv(name))
		return val;
	else
		return std::string();
}

inline
void set(const char* name, const char* value, bool allowOverwrite = true) {
	setenv(name, value, allowOverwrite);
}

inline
void unset(const char* name) {
	unsetenv(name);
}

/// Replaces $<single-char-name> or ${multi-char-name} with the corresponding environment variables' value
std::string replace(const char* const beg, const char* const end) {
	using namespace std;

	assert(beg <= end);

	const char* dollar;
	const char* name_beg;
	const char* name_end;
	const char* last_end = beg;

	char nameBuffer[128];

	std::string result;
	while(true) {
		dollar = std::find(last_end, end, '$');
		result.append(last_end, dollar);
		if(dollar >= end)
			break;
		if(dollar[1] == '{') {
			name_beg = dollar + 2;
			name_end = std::find(name_beg, end, '}');
			if(name_end >= end) {
				throw std::runtime_error("Expected closing brace");
			}

			last_end = name_end + 1;
		}
		else {
			name_beg = dollar + 1;
			name_end = name_beg + 1;
			last_end = name_end;
			if(name_end > end) {
				throw std::runtime_error("Dollar $ in environment replacement is at the end of the string");
			}
		}

		if(name_end - name_beg <= 127) {
			// We can use our small nameBuffer to prevent allocations
			std::copy(name_beg, name_end, nameBuffer)[0] = '\0';
			if(const char* value = get(nameBuffer))
				result += value;
		}
		else {
			std::string name = std::string(name_beg, name_end);
			if(const char* value = get(name.c_str()))
				result += value;
		}
	}

	return result;
}

std::string replace(const char* const beg) {
	return replace(beg, beg + strlen(beg));
}
std::string replace(std::string const& s) {
	return replace(s.data(), s.data() + s.size());
}
template<size_t N>
std::string replace(char const (&s)[N]) {
	return replace(s, s + N);
}

}} // namespace stx::env

#endif // header guard STX_ENVIRONMENT_HPP_INCLUDED

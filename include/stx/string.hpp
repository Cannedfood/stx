// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#ifndef STX_STRING_INCLUDED
#define STX_STRING_INCLUDED

#include <cstring>
#include <stdexcept>

namespace stx {

class token {
	const char* m_begin;
	size_t      m_size;
public:
	constexpr
	token() noexcept :
		m_begin(nullptr),
		m_size(0)
	{}

	constexpr
	token(const char* beg, size_t len) noexcept :
		m_begin(beg),
		m_size(len)
	{}
	constexpr
	token(const char* beg, const char* end) noexcept :
		m_begin(beg),
		m_size(end - beg)
	{}

	token(const char* beg) noexcept : token(beg, strlen(beg)) {}

	using iterator   = const char*;
	using value_type = char;

	constexpr size_t      size() const noexcept { return m_size; }
	constexpr bool        empty() const noexcept { return size() == 0; }
	constexpr const char* begin() const noexcept { return m_begin; }
	constexpr const char* end() const noexcept { return begin() + size(); }

	constexpr operator bool() const noexcept { return begin(); }
	constexpr const char& operator[](size_t i) const { return begin()[i]; }

	constexpr
	int comp(token const& other) const noexcept {
		const char* begA = begin();
		const char* begB = other.begin();
		const char* endA = end();
		const char* endB = other.end();

		while(true) {
			begA++;
			begB++;
			if(begA >= endA) {
				return begB >= endB ? 0 : -1;
			}
			if(begB >= endB) {
				return 1;
			}
			int dif = *begA - *begB;
			if(dif != 0) {
				return dif < 0 ? -1 : 1;
			}
		}
	}
	constexpr
	bool operator==(token const& other) const noexcept {
		if(other.size() != size()) return false;
		for(size_t i = 0; i < size(); i++) {
			if(m_begin[i] != other.m_begin[i]) return false;
		}
		return true;
	}
	constexpr
	bool operator<(token const& other) const noexcept {
		return comp(other) < 0;
	}

	token trim() const noexcept {
		const char* beg = m_begin;
		const char* end = m_begin + m_size;
		while(beg < end && isblank(*beg))
			beg++;
		while(end > beg && isblank(end[-1])) {
			--end;
		}
		return token(beg, end);
	}

	template<class Pred> constexpr
	token substring(Pred&& pred) const {
		const char* end = this->end();
		const char* substr_end = begin();
		while(substr_end < end) {
			if(!pred(*substr_end)) {
				return token(begin(), substr_end);
			}
		}
		return *this;
	}
};

class string : public token {
public:
	string(const char* beg, size_t len) noexcept :
		token(new char[len + 1], len)
	{
		memcpy((char*)begin(), beg, len);
		const_cast<char&>(*end()) = '\0';
	}

	string(const char* beg, const char* end) noexcept :
		string(beg, end - beg)
	{}

	string(token tok) noexcept :
		string(tok.begin(), tok.end())
	{}

	string(const char* beg) noexcept :
		string(beg, strlen(beg))
	{}

	~string() noexcept {
		if(*this)
			delete[] begin();
	}

	using iterator       = char*;
	using const_iterator = const char*;

	constexpr char* begin() noexcept { return const_cast<char*>(token::begin()); }
	constexpr char* end()   noexcept { return const_cast<char*>(token::end()); }
	constexpr char const* begin() const noexcept { return token::begin(); }
	constexpr char const* end()   const noexcept { return token::end(); }
};

} // namespace stx

#endif // headguard STX_STRING_INCLUDED

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

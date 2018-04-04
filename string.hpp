// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#ifndef STX_STRING_INCLUDED
#define STX_STRING_INCLUDED

#include <cstring> // strlen
#include <cctype> // isspace(int)
#include <cstdlib> // mblen(...) mbtowc(...)
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace stx {

class token {
	const char* m_begin;
	size_t      m_size;
public:
	constexpr
	token() noexcept :
		m_begin(""),
		m_size(0)
	{}

	template<size_t N> constexpr
	token(const char (&str)[N]) :
		m_begin(str),
		m_size(N - 1)
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

	constexpr
	void clear() {
		reset();
	}

	constexpr
	void reset() {
		reset("", size_t(0));
	}

	constexpr
	void reset(const char* begin, const char* end) {
		m_begin = begin;
		m_size = end - begin;
	}

	constexpr
	void reset(const char* begin, size_t len) {
		m_begin = begin;
		m_size  = len;
	}

	void reset(const char* str) {
		m_begin = str;
		m_size  = strlen(str);
	}

	template<size_t N> constexpr
	void reset(const char (&str)[N]) {
		m_begin = str;
		m_size  = N - 1;
	}

	using iterator   = const char*;
	using value_type = char;

	constexpr size_t      size() const noexcept { return m_size; }
	constexpr bool        empty() const noexcept { return size() == 0; }
	constexpr const char* begin() const noexcept { return m_begin; }
	constexpr const char* end() const noexcept { return begin() + size(); }

	size_t length() const noexcept {
		size_t count = 0;
		iterator ptr = begin();
		while(ptr < end()) {
			ptr += mblen(ptr, end() - ptr);
			count++;
		}
		return count;
	}

	std::vector<wchar_t> to_wchar() {
		std::vector<wchar_t> result;
		result.reserve(size());

		iterator ptr = begin();
		while(ptr < end()) {
			result.emplace_back();
			int c = mbtowc(&result.back(), ptr, end() - ptr);
			if(c < 0) throw std::runtime_error("Invalid utf8 character");
			ptr += c;
		}

		return result;
	}

	constexpr operator bool() const noexcept { return !empty(); }
	constexpr const char& operator[](size_t i) const { return begin()[i]; }

	constexpr
	int compare(token const& other) const noexcept {
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
		return compare(other) < 0;
	}

	bool operator==(const char* s) const noexcept {
		return *this == token(s);
	}
	bool operator<(const char* s) const noexcept {
		return compare(token(s)) < 0;
	}

	template<size_t N> constexpr
	bool operator==(const char (&s)[N]) const noexcept {
		return *this == token(s);
	}
	template<size_t N> constexpr
	bool operator<(const char (&s)[N]) const noexcept {
		return compare(token(s)) < 0;
	}

	template<class BegPred, class EndPred>
	token trim(BegPred&& begPred, EndPred&& endPred) const noexcept {
		const char* beg = m_begin;
		const char* end = m_begin + m_size;
		while(beg < end && begPred(*beg))
			beg++;
		while(end > beg && endPred(end[-1])) {
			--end;
		}
		return token(beg, end);
	}

	template<class Pred = int(* const&)(int)>
	token trim(Pred&& pred = isspace) const noexcept {
		return trim<Pred, Pred>(pred, pred);
	}

	template<class Pred>
	token trim_front(Pred&& pred) const noexcept {
		return trim<Pred, Pred>(pred, pred);
	}

	template<class Pred>
	token trim_back(Pred&& pred) const noexcept {
		return trim<Pred, Pred>(pred, pred);
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

class mutable_token : public token {
public:
	using token::token; // Inherit constructors

	constexpr explicit
	mutable_token(token t) :
		token(t)
	{}

	using iterator       = char*;
	using const_iterator = const char*;

	constexpr char* begin() noexcept { return const_cast<char*>(token::begin()); }
	constexpr char* end()   noexcept { return const_cast<char*>(token::end()); }
	constexpr char const* begin() const noexcept { return token::begin(); }
	constexpr char const* end()   const noexcept { return token::end(); }
};

class string : public mutable_token {
public:
	string() :
		mutable_token()
	{}

	string(const char* beg, size_t len) noexcept :
		mutable_token(
			(char*) memcpy(new char[len + 1], beg, len), len)
	{
		*end() = '\0';
	}

	template<size_t N>
	string(const char (&str)[N]) :
		string(str, N)
	{}

	string(const char* beg, const char* end) noexcept :
		string(beg, end - beg)
	{}

	string(token tok) noexcept :
		string(tok.begin(), tok.size())
	{}

	string(size_t num_repetitions, token t) :
		string()
	{
		size_t size = t.size() * num_repetitions;
		mutable_token::reset(new char[t.size() * num_repetitions + 1], size);
		iterator to = begin();
		for (size_t i = 0; i < num_repetitions; i++) {
			memcpy(to, t.begin(), t.size());
			to += t.size();
		}
	}

	explicit
	string(const char* beg) noexcept :
		string(beg, strlen(beg))
	{}

	~string() noexcept {
		if(!empty())
			delete[] begin();
	}

	string(std::initializer_list<token> tokens) :
		mutable_token()
	{
		size_t totalSize = 0;
		for(auto& t : tokens)
			totalSize += t.size();
		reset(new char[totalSize + 1], totalSize + 1);
	}

	constexpr
	void clear() {
		reset();
	}

	constexpr
	void reset() {
		mutable_token::reset("", size_t(0));
	}

	void reset(const char* begin, const char* end) {
		reset(begin, end - begin);
	}

	void reset(const char* begin, size_t len) {
		if(len == 0) {
			token::reset();
		}
		else {
			token::reset(
				(char*) memcpy(new char[len + 1], begin, len), len
			);
			*end() = '\0';
		}
	}

	void reset(const char* str) {
		reset(str, strlen(str));
	}

	template<size_t N>
	void reset(const char (&str)[N]) {
		reset(str, N - 1);
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

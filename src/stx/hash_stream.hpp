#pragma once

#include <iostream>
#include <ostream>
#include <streambuf>
#include <string>

#include "hash.hpp"

namespace stx {

template<class Hasher = stx::hasher, class CharT = char, class Traits = std::char_traits<CharT>>
class hash_streambuf : public std::basic_streambuf<CharT, Traits> {
public:
	Hasher hash;

	hash_streambuf(Hasher seed = {}) noexcept : hash(std::move(seed)) {}

protected:
	using int_type = typename Traits::int_type;

	std::streamsize xsputn(const CharT* s, std::streamsize n) override {
		hash(s, n);
		return n;
	}

	int_type overflow(int_type c) override {
		const char cc = c;
		hash(&cc);
		return c;
	}
};

template<class Hasher = stx::hasher, class CharT = char, class Traits = std::char_traits<CharT>>
class hash_ostream : public hash_streambuf<Hasher>, public std::basic_ostream<CharT, Traits> {
public:
	hash_ostream(Hasher seed = {}) noexcept :
		hash_streambuf<Hasher>(std::move(seed)),
		std::ostream((std::streambuf*)this)
	{}
};

} // namespace stx

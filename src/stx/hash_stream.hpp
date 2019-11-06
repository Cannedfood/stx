#pragma once

#include <iostream>
#include <ostream>
#include <streambuf>

#include "hash.hpp"


namespace stx {

template<class Hasher = stx::hasher>
class hash_streambuf : public std::streambuf {
public:
	Hasher hash;

	hash_streambuf(Hasher seed = {}) noexcept : hash(std::move(seed)) {}

protected:
	std::streamsize xsputn(const char_type* s, std::streamsize n) override {
		hash(s, n);
	}

	int_type overflow(int_type c) override {
		const char cc = c;
		hash(&cc);
	}
};

template<class Hasher = stx::hasher>
class hash_ostream : public std::ostream, public hash_streambuf<Hasher> {
public:
	hash_ostream(Hasher seed = {}) noexcept :
		hash_streambuf<Hasher>(std::move(seed)),
		std::ostream((std::streambuf*)this)
	{}
};

} // namespace stx

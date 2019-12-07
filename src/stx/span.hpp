#pragma once

#include <cstddef>
namespace stx {

template<class T>
class span {
public:
	span() noexcept : _begin(nullptr), _count(0) {}
	span(T* ptr, size_t count) noexcept : _begin(ptr), _count(count) {}
	span(T* ptr, T* end) noexcept : _begin(ptr), _count(end - ptr) {}

	using iterator = T;
	T* begin() { return _begin; }
	T* end()   { return _begin + _count; }

	T&       operator[](size_t i)       noexcept { return _begin[i]; }
	T const& operator[](size_t i) const noexcept { return _begin[i]; }
private:
	T*     _begin;
	size_t _count;
};

} // namespace stx

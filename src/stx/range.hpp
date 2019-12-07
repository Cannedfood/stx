#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace stx {

template<class T>
struct iter_range_t {
	T _begin, _end;

	T& begin() { return _begin; }
	T& end() { return _end; }
};

template<class T>
auto range(T&& start, T&& end) -> iter_range_t<std::remove_reference_t<T>> { return {start, end}; }

template<class T>
auto range(T&& start, size_t count) -> iter_range_t<std::remove_reference_t<T>> { return {start, start + count}; }

template<class CastTo, class SubIter>
class cast_iterator {
public:
	std::remove_const_t<SubIter> iterator;

	cast_iterator& operator++() {
		++iterator;
		return *this;
	}

	CastTo operator*() {
		return (CastTo) *iterator;
	}

	bool operator!=(cast_iterator const& other) const {
		return iterator != other.iterator;
	}
};

template<class CastTo, class T>
auto cast_range(T&& start, T&& end) -> iter_range_t<cast_iterator<CastTo, std::remove_reference_t<T>>> { return {{start}, {end}}; }

template<class CastTo, class T>
auto cast_range(T&& start, size_t count) -> iter_range_t<cast_iterator<CastTo, std::remove_reference_t<T>>> { return {{start}, {start + count}}; }



template<class T>
auto bytes(T* start, T* end) -> iter_range_t<T*> { return {(uint8_t*) start, (uint8_t*) end}; }

template<class T>
auto bytes(T* start, size_t count) -> iter_range_t<uint8_t*> { return {(uint8_t*) start, (uint8_t*) (start + count)}; }

template<class T>
auto bytes(T const& t) -> iter_range_t<uint8_t const*> { return {(uint8_t const*)&t, ((uint8_t const*)&t) + sizeof(T)}; }

template<class T>
auto bytes(T&& t) -> iter_range_t<uint8_t*> { return {(uint8_t*)&t, ((uint8_t*)&t) + sizeof(T)}; }

template<class T>
uint8_t* bytes_begin(T& t) noexcept { return (uint8_t*) &t; }

template<class T>
uint8_t* bytes_end(T& t) noexcept { return ((uint8_t*) ((&t) + 1)); }

} // namespace stx

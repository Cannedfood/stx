#pragma once

namespace stx {

template<class T>
struct iter_range_t {
	T _begin, _end;

	T& begin() { return _begin; }
	T& end() { return _end; }
};

template<class T>
auto range(T& start, T&& end) -> iter_range_t<T> { return {start, end}; }

/*
template<class T>
struct value_range_t {

};
*/

} // namespace stx

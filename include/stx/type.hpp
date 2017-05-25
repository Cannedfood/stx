#pragma once

namespace stx {

namespace detail {

template<typename T>
struct pointer_to_struct {
	using type = T*;
};

template<typename T>
struct pointer_to_struct<T[]> {
	using type = T*;
};

} // namespace detail

template<typename T>
using pointer_to = typename detail::pointer_to_struct<T>::type;

template<typename T>
struct default_delete {
	using pointer = pointer_to<T>;

	inline
	void operator()(pointer p) const noexcept {
		delete p;
	}
};

} // namespace stx

#pragma once

#include <type_traits>
#include <functional>

namespace stx {

template<class B>
class scope {
	std::remove_reference_t<B> m_whenOutOfScope;

public:
	scope(B&& b) : m_whenOutOfScope(std::forward<B>(b)) {}

	template<class A>
	scope(A&& a, B&& b) : m_whenOutOfScope(std::forward<B>(b)) {
		a();
	}

	~scope() { m_whenOutOfScope(); }
};

} // namespace stx

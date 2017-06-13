#pragma once

#include <utility>

#ifdef STX_DEBUG
#	include "assert.hpp"
#endif

namespace stx {

template<typename T>
class pointer_pair {
	using Tself  = pointer_pair<T>;
	using Tother = T;

	T* m_other;

public:
	pointer_pair(Tself const&) = delete;

	pointer_pair() : m_other(nullptr) {}
	pointer_pair(Tself&& other) :
		pointer_pair()
	{
		*this = std::move(other);
	}

	~pointer_pair() noexcept {
		reset();
	}

	Tself& operator=(Tself const&) = delete;
	Tself& operator=(Tself&& other) noexcept {
		reset(other.m_other);
		return *this;
	}

	void reset(T* other = nullptr) noexcept {
		if(m_other) {
#ifdef STX_DEBUG
			xassert(m_other->m_other == this);
#endif
			m_other->m_other = nullptr;
		}

		m_other = other;
		if(m_other) {
			m_other->reset();
			m_other->m_other = (decltype(m_other->m_other)) this;
		}
	}

	T* get() noexcept { return m_other; }
};

} // namespace stx

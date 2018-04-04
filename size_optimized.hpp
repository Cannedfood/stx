#pragma once

#include <type_traits>
#include <utility>

namespace stx {

template<class T, unsigned size = 128 - sizeof(T*)>
struct size_optimized {
	char m_optimization[size];
	T*   m_value;
public:
	using self_t = size_optimized<T, size>;

	// Not copy- or movable
	size_optimized(self_t const& other) = delete;
	size_optimized& operator=(self_t const& other) = delete;
	size_optimized(self_t&& other) = delete;
	size_optimized& operator=(self_t&& other) = delete;

	size_optimized() : m_value(nullptr) {}

	template<class Tx>
	size_optimized<T, size>& operator=(Tx&& tx) {
		using basic_Tx = std::remove_reference_t<std::remove_cv_t<Tx>>;

		reset();

		if constexpr(sizeof(tx) < m_optimization) {
			m_value = new(m_optimization) basic_Tx(std::forward<Tx>(tx));
		}
		else {
			static_assert(std::has_virtual_destructor_v<T>, "Objects needs virtual destructor");
			m_value = new basic_Tx(std::forward<Tx>(tx));
		}
	}

	void reset() {
		if(m_value) {
			if(m_value >= m_optimization && m_value <= m_optimization + sizeof(m_optimization)) {
				m_value->~T();
			}
			else {
				delete m_value;
			}
			m_value = nullptr;
		}
	}

	T* operator->() { return m_value; }
};

} // namespace stx

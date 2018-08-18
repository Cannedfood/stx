#pragma once

#include <type_traits>
#include <functional>

namespace stx {

template<class T>
class scoped_action {
	std::remove_reference_t<T> m_execute;
public:
	scoped_action(T&& t) : m_execute(t) {}
	~scoped_action() { m_execute(); }
};

template<class T>
class scoped_action<std::function<T>> : public std::function<T> {
public:
	using std::function<T>::function;
	~scoped_action() {
		if(*this) {
			(*this)();
		}
	}
};

} // namespace stx

#ifndef STX_SHARED_PTR_HPP_INCLUDED
#define STX_SHARED_PTR_HPP_INCLUDED

#pragma once

#include <atomic>

namespace stx {

template<class T>
class shared {
	T* m_value;
public:
};

template<class T, class... Args>
shared<T> make_shared(Args&&... args) {

}

} // namespace stx

#endif // header guard STX_SHARED_PTR_HPP_INCLUDED
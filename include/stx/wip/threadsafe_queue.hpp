#pragma once

#include "xqueue.hpp"
#include "xmutex.hpp"

namespace stx {

template<typename T>
class threadsafe_queue {
	::stx::mutex    m_mutex;
	::stx::queue<T> m_queue;
};

} // namespace stx

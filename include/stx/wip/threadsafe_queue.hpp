#pragma once

#include <mutex>
#include <queue>

namespace stx {

template<typename T>
class threadsafe_queue {
	::std::mutex    m_mutex;
	::std::queue<T> m_queue;
};

} // namespace stx

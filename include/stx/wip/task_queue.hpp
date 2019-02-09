#pragma once

#include <functional>
#include <deque>
#include <mutex>

namespace stx {

class task_queue {
	std::mutex                        m_mutex;
	std::deque<std::function<void()>> m_tasks;
public:
	void defer(std::function<void()> task);
	void execute_tasks();
};

} // namespace stx
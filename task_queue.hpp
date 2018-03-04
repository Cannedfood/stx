#ifndef STX_TASK_QUEUE_HPP_INCLUDED
#define STX_TASK_QUEUE_HPP_INCLUDED

#pragma once

#include <functional>
#include <mutex>
#include <atomic>
#include <deque>
#include <map>

namespace stx {

class error : public std::exception_ptr {
public:
	error() : exception_ptr(std::current_exception()) {}

	error(std::nullptr_t) : exception_ptr(nullptr) {}
};

class task_queue {
	std::mutex                        m_mutex;
	std::deque<std::function<void()>> m_tasks;
public:
	void defer(std::function<void()> task);
	void execute_tasks();
};

class advanced_task_queue {
	std::mutex                                  m_mutex;
	std::multimap<float, std::function<void()>> m_tasks;
public:
	void defer(std::function<void()> task, float priority = 0);
	void execute_tasks();
};

} // namespace stx

#endif // header guard STX_TASK_QUEUE_HPP_INCLUDED

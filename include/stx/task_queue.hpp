#ifndef STX_TASK_QUEUE_HPP_INCLUDED
#define STX_TASK_QUEUE_HPP_INCLUDED

#pragma once

#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <thread>
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
	using container_t = std::multimap<double, std::function<void()>>;

	bool                    m_quitting;
	container_t             m_tasks;
	std::mutex              m_mutex;
	std::condition_variable m_signal;

	std::vector<std::thread> m_threads;
public:
	advanced_task_queue();
	~advanced_task_queue();

	void defer(std::function<void()> task, double priority = 0);
	void execute_tasks();
	void keep_executing_tasks();

	void run(size_t count);
	void stop();
};

} // namespace stx

#endif // header guard STX_TASK_QUEUE_HPP_INCLUDED

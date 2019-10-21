#pragma once

#include "../async.hpp"

#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <map>

namespace stx {

class task_queue : public executor {
public:
	void defer(std::function<void()> task, float priority = 0) noexcept;
	bool execute_tasks() noexcept;
	void start() noexcept;

private:
	std::mutex                                  m_mutex;
	std::multimap<float, std::function<void()>> m_tasks;
};

class task_queue_mt : public executor {
public:
	task_queue_mt() noexcept;
	~task_queue_mt() noexcept;

	void defer(std::function<void()> task, float priority = 0) noexcept;
	void execute_tasks() noexcept;
	void start() noexcept;
	void stop()  noexcept;

private:
	volatile bool                               m_finish;
	std::atomic<int>                            m_num_threads;

	std::multimap<float, std::function<void()>> m_tasks;

	std::mutex                                  m_mutex;
	std::condition_variable                     m_sleeping_threads;
};

} // namespace stx
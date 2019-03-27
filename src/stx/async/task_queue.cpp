#include "task_queue.hpp"

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace stx {

// =============================================================
// == task_queue =============================================
// =============================================================

void task_queue::defer(std::function<void()> task, float priority) noexcept {
	std::scoped_lock lock{m_mutex};

	m_tasks.emplace(priority, std::move(task));
}

void task_queue::execute_tasks() noexcept {
	std::multimap<float, std::function<void()>> tasks;
	while(m_tasks.size()) {
		// Get tasks
		{ std::scoped_lock lock{m_mutex};
			std::swap(m_tasks, tasks);
		}

		// Execute tasks
		for(auto& [prio, task] : tasks) {
			// Move task out of queue and execute
			std::function<void()>(std::move(task))();
		}

		// Clear executed tasks
		tasks.clear();
	}
}

void task_queue::start() noexcept {
	// TODO
}

// =============================================================
// == task_queue_mt =============================================
// =============================================================

task_queue_mt::task_queue_mt() noexcept :
	m_finish(false),
	m_num_threads(0)
{}
task_queue_mt::~task_queue_mt() noexcept {
	stop();
}

void task_queue_mt::defer(std::function<void()> fn, float prio) noexcept {
	{ std::scoped_lock lock{m_mutex};
		m_tasks.emplace(prio, std::move(fn));
	}
	m_sleeping_threads.notify_one();
}
void task_queue_mt::execute_tasks() noexcept {
	++m_num_threads;

	std::function<void()> task;

	while(true) {
		std::unique_lock lock{m_mutex};

		// See if we have a task
		if(m_tasks.empty()) break;

		// Get the task
		task = std::move(m_tasks.begin()->second);
		m_tasks.erase(m_tasks.begin());

		// Execute the task
		task();
	}

	--m_num_threads;
}
void task_queue_mt::start() noexcept {
	m_finish = false;

	++m_num_threads;

	std::function<void()> task;

	while(true) {
		{
			std::unique_lock lock{m_mutex};

			// Wait until we have a task
			if(m_tasks.empty()) {
				m_sleeping_threads.wait(lock, [this]() { return m_finish || !m_tasks.empty(); });
				if(m_tasks.empty() && m_finish) break;
			}

			// Get the task
			task = std::move(m_tasks.begin()->second);
			m_tasks.erase(m_tasks.begin());
		}

		// Execute the task
		task();
	}

	--m_num_threads;
}
void task_queue_mt::stop() noexcept {
	m_finish = true;

	m_sleeping_threads.notify_all(); // Wake up all threads, so they can be joined
	execute_tasks(); // Execute all pending tasks
	while(m_num_threads > 0); // Spin until all threads are finished
}

} // namespace stx

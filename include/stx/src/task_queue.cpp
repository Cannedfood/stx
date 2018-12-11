#include "../task_queue.hpp"

#include <chrono>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace stx {

void task_queue::defer(std::function<void()> task) {
	m_mutex.lock();
	m_tasks.emplace_back(std::move(task));
	m_mutex.unlock();
}

void task_queue::execute_tasks() {
	std::deque<std::function<void()>> tasks;
	while(m_tasks.size()) {
		m_mutex.lock();
		std::swap(m_tasks, tasks);
		m_mutex.unlock();
		for(auto& t : tasks) {
			if(t) t();
		}
		tasks.clear();
	}
}

advanced_task_queue::advanced_task_queue() :
	m_quitting(false)
{}

advanced_task_queue::~advanced_task_queue() {
	stop();
}

void advanced_task_queue::defer(std::function<void()> task, double priority) {
	m_tasks.emplace(priority, std::move(task));
	m_signal.notify_one();
}

void advanced_task_queue::execute_tasks() {
	m_mutex.lock();
	while(!m_tasks.empty()) {
		auto node = m_tasks.extract(m_tasks.begin());
		m_mutex.unlock();
		try {
			node.mapped()(); // Execute task
		}
		catch(...) {
			m_mutex.unlock();
			throw;
		}
		m_mutex.lock();
		// Node destroyed
	}
	m_mutex.unlock();
}

void advanced_task_queue::keep_executing_tasks() {
	while(!m_quitting) {
		container_t::node_type node;

		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if(m_tasks.empty()) {
				m_signal.wait(lock, [&]() -> bool {
					return !m_tasks.empty() || m_quitting;
				});

				if(!m_tasks.empty()) {
					if(m_quitting) return;
					else continue;
				}
			}

			node = m_tasks.extract(m_tasks.begin());
		}

		node.mapped()();
	}
}

void advanced_task_queue::run(size_t count) {
	for(size_t i = 0; i < count; i++) {
		m_threads.emplace_back(std::thread([this]() {
			keep_executing_tasks();
		}));
	}
}

void advanced_task_queue::stop() {
	m_quitting = true;
	m_signal.notify_all();
	for(auto& t : m_threads) {
		t.join();
	}
}

} // namespace stx

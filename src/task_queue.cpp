#include "../task_queue.hpp"

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

void advanced_task_queue::defer(std::function<void()> task, float priority) {
	m_tasks.emplace(priority, std::move(task));
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

} // namespace stx

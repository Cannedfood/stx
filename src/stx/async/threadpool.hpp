#pragma once

#include "task_queue.hpp"

#include <thread>

namespace stx {

class threadpool : public task_queue_mt {
	std::vector<std::thread> m_threads;
public:
	threadpool() noexcept {}
	threadpool(int count) noexcept {
		start(count);
	}

	~threadpool() noexcept { stop(); }

	void start(int count = 0) noexcept {
		stop();

		if(count <= 0) {
			count = std::thread::hardware_concurrency() - 1;
		}

		for(unsigned i = 0; i < count; i++) {
			m_threads.emplace_back([this]() {
				task_queue_mt::start();
			});
		}
	}

	void stop() noexcept {
		task_queue_mt::stop();
		for(auto& thread : m_threads) {
			thread.join();
		}
		m_threads.clear();
	}
};

} // namespace stx

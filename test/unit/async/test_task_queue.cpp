#include "../catch.hpp"

#include <stx/async/task_queue.hpp>
using namespace stx;

#include <chrono>
using namespace std::chrono;
using namespace std::chrono_literals;
#include <thread>

auto THREADING_STRESSTEST_DURATION = 30ms;

TEST_CASE("Test task_queue", "[task_queue]") {
	task_queue queue;

	SECTION("All tasks are executed") {
		int n = 10;
		for(int i = 0; i < 10; i++) {
			queue.defer([&]() { n--; });
		}
		queue.execute_tasks();
		CHECK(n == 0);
	}

	SECTION("Adding tasks in parallel") {
		std::atomic<int> balance{0};

		auto end = steady_clock::now() + THREADING_STRESSTEST_DURATION;

		std::vector<std::thread> threads;
		for(size_t i = 0; i < 100; i++) {
			threads.emplace_back([&]() {
				while(steady_clock::now() < end) {
					balance++;
					queue.defer([&]() { balance--; });
				}
			});
		}

		while(steady_clock::now() < end) {
			queue.execute_tasks();
		}

		for(auto& thread : threads) thread.join();
		queue.execute_tasks();
		CHECK(balance == 0);
	}
}

TEST_CASE("Test task_queue_mt", "[task_queue]") {
	SECTION("All tasks are executed") {
		task_queue_mt queue;
		int n = 10;
		for(int i = 0; i < 10; i++) {
			queue.defer([&]() { n--; });
		}
		queue.execute_tasks();
		CHECK(n == 0);
	}

	SECTION("Adding tasks in parallel") {
		task_queue_mt queue;

		std::atomic<int> balance{0};

		auto end = steady_clock::now() + THREADING_STRESSTEST_DURATION;

		std::vector<std::thread> threads;
		for(size_t i = 0; i < 100; i++) {
			threads.emplace_back([&]() {
				while(steady_clock::now() < end) {
					balance++;
					queue.defer([&]() { balance--; });
				}
			});
		}

		while(steady_clock::now() < end) {
			queue.execute_tasks();
		}

		for(auto& thread : threads) thread.join();
		queue.execute_tasks();
		CHECK(balance == 0);
	}

	SECTION("Adding tasks & executing in parrallel") {
		std::atomic<int> balance{0};

		std::vector<std::thread> threads;

		{
			task_queue_mt queue;

			// Start queue as thread pool
			for(size_t i = 0; i < 10; i++) {
				threads.emplace_back([&]() {
					queue.start();
				});
			}

			// Schedule adding tasks for 100ms
			auto end = steady_clock::now() + THREADING_STRESSTEST_DURATION;
			for(size_t i = 0; i < 100; i++) {
				threads.emplace_back([&]() {
					while(steady_clock::now() < end) {
						balance++;
						queue.defer([&]() { balance--; });
					}
				});
			}

			std::this_thread::sleep_for(THREADING_STRESSTEST_DURATION);
		}

		for(auto& thread : threads) {
			thread.join();
		}

		CHECK(balance == 0);
	}
}
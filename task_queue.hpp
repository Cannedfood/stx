#ifndef STX_TASK_QUEUE_HPP_INCLUDED
#define STX_TASK_QUEUE_HPP_INCLUDED

#pragma once

#include <functional>
#include <mutex>
#include <atomic>
#include <deque>
#include <map>

namespace stx {

class task_queue {
	std::mutex                        mMutex;
	std::deque<std::function<void()>> mTasks;
public:
	void defer(std::function<void()> task) {
		mMutex.lock();
		mTasks.emplace_back(std::move(task));
		mMutex.unlock();
	}

	void execute_tasks() {
		std::deque<std::function<void()>> tasks;
		while(mTasks.size()) {
			mMutex.lock();
			std::swap(mTasks, tasks);
			mMutex.unlock();
			for(auto& t : tasks) {
				if(t) t();
			}
			tasks.clear();
		}
	}
};

class advanced_task_queue {
	std::mutex                                  mMutex;
	std::multimap<float, std::function<void()>> mTasks;
public:
	void defer(std::function<void()> task, float priority = 0) {

	}

	void execute_tasks() {
		mMutex.lock();
		while(!mTasks.empty()) {
			auto node = mTasks.extract(mTasks.begin());
			mMutex.unlock();
			try {
				node.mapped()(); // Execute task
			}
			catch(...) {
				mMutex.unlock();
				throw;
			}
			mMutex.lock();
			// Node destroyed
		}
		mMutex.unlock();
	}
};

} // namespace stx

#endif // header guard STX_TASK_QUEUE_HPP_INCLUDED

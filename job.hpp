#ifndef STX_JOB_HPP_INCLUDED
#define STX_JOB_HPP_INCLUDED

#pragma once

#include <atomic>
#include <cstddef>

namespace stx {

class task {
public:
	virtual void start()   = 0;
	virtual bool started() = 0;
	virtual void cancel()  = 0;

	virtual void finish()   = 0;
	virtual bool finished() = 0;

private:
	friend class job;
	task* m_job_next;
};

// TODO: make thread safe (atomic)
class job {
public:
	template<class Executor, class Function>
	void add(Executor&& ex, Function&& fn) { push_task(ex.add(fn)); }

	void cancel();
	void finish();
private:
	void  push_task(task* task);
	task* pop_task();

	std::atomic<task*> m_first_task;
};

} // namespace stx

#endif // header guard STX_JOB_HPP_INCLUDED
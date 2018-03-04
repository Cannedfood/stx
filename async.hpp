#include "handle.hpp"

#include <deque>
#include <mutex>

namespace stx {

class task : public stx::handle_slot, public stx::list_element_mt<task> {
public:
	task() {}
};

/*
template<class Callable>
class task_impl : public task {
	Callable m_callable;
public:
};
*/

class task_queue {
	std::mutex m_mutex;
	stx::list_mt<task> m_tasks;
public:
	task_queue() {}

	handle_slot& add(task* t) {
		return *t;
	}

	void run() {

	}
};

} // namespace stx

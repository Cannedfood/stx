#include "../unmaintained/job.hpp"

namespace stx {

void job::cancel() {
	while(task* t = pop_task()) t->cancel();
}
void job::finish() {
	while(task* t = pop_task()) t->finish();
}

void job::push_task(task* added_task) {
	task* tmp_first_task;
	do {
		tmp_first_task = m_first_task;
		added_task->m_job_next = m_first_task;
		// TODO: does the order of tasks in a job really not matter? (std::memory_order_relaxed ok?)
	} while(m_first_task.compare_exchange_weak(tmp_first_task, added_task, std::memory_order_relaxed));;
}
task* job::pop_task() {
	task* first;
	while(
		(first = m_first_task) &&
		!m_first_task.compare_exchange_weak(
				first, first->m_job_next,
				std::memory_order_relaxed
			)
	);
	return first;
}

} // namespace stx
#include "async.hpp"
#include "async/threadpool.hpp"

#include <thread>
#include <memory>

namespace stx {

executor& global_threadpool() noexcept {
	static std::unique_ptr<threadpool> p_threadpool = nullptr;

	if(!p_threadpool) {
		unsigned count = std::thread::hardware_concurrency() - 1;

		p_threadpool = std::make_unique<threadpool>(count);
	}

	return *p_threadpool;
}
executor& global_io_thread()  noexcept {
	static std::unique_ptr<threadpool> p_io_thread  = nullptr;

	if(!p_io_thread) {
		p_io_thread = std::make_unique<threadpool>(1);
	}

	return *p_io_thread;
}

} // namespace stx

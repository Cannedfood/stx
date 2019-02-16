#include "../async.hpp"
#include "../environment.hpp"
#include "../async/threadpool.hpp"

#include <thread>
#include <memory>

namespace stx {

namespace {

static std::unique_ptr<threadpool> p_threadpool = nullptr;
static std::unique_ptr<threadpool> p_io_thread  = nullptr;

} // namespace

executor& global_threadpool() noexcept {
	if(!p_threadpool) {
		unsigned count = std::thread::hardware_concurrency() - 1;

		if(const char* value = stx::env::get("STX_THREADPOOL_SIZE")) {
			count = strtoul(value, nullptr, 10);
		}

		p_threadpool = std::make_unique<threadpool>(count);
	}

	return *p_threadpool;
}
executor& global_io_thread()  noexcept {
	if(!p_io_thread) {
		p_io_thread = std::make_unique<threadpool>(1);
	}
	return *p_io_thread;
}

} // namespace stx

#include "../async.hpp"
#include "../environment.hpp"
#include "../async/threadpool.hpp"

#include <thread>

namespace stx {

namespace {

static threadpool g_threadpool{std::thread::hardware_concurrency() / 2};
static threadpool g_io_thread{1};

} // namespace

executor& global_threadpool() noexcept { return g_threadpool; }
executor& global_io_thread()  noexcept { return g_io_thread; }

} // namespace stx

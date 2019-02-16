#pragma once

#include <functional>

namespace stx {

// ** Forward declarations *******************************************************
template<class T> class weak;

// ** Executor *******************************************************
// TODO: promises
class executor {
public:
	virtual void defer(std::function<void()> fn, float priority = 0) noexcept { fn(); }

	template<class Callback, class T>
	void defer(Callback&& callback, weak<T> context, float priority) noexcept;

	virtual ~executor() {}
};

// ** Standard Executors *******************************************************
executor& global_threadpool()  noexcept;
executor& global_io_thread()   noexcept;

} // namespace stx



// =============================================================
// == Inline Implementation =============================================
// =============================================================

namespace stx {

template<class Callback, class T>
void executor::defer(Callback&& callback, weak<T> context, float priority) noexcept {
	defer(
		[cb = std::move(callback), ctxt = std::move(context)]() {
			if(auto tmp = ctxt.lock()) {
				cb();
			}
		},
		priority
	);
}

} // namespace stx
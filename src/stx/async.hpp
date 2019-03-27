#pragma once

#include <functional>

namespace stx {

// ** Forward declarations *******************************************************
template<class T> class weak;

// ** Executor *******************************************************
// TODO: promises
class executor {
public:
	inline virtual ~executor() {}

	inline virtual void defer(std::function<void()> fn, float priority = 0) noexcept { fn(); }

	template<class Callback, class T>
	void defer(Callback&& callback, weak<T> context, float priority) noexcept;

	template<class... Args> inline
	void operator()(Args&&... args) noexcept { defer(std::forward<Args>(args)...); }
};

// ** Standard Executors *******************************************************
executor& global_threadpool()  noexcept;
executor& global_io_thread()   noexcept;

// ** Utilities ***************************************************************
template<class T>
auto assignTo(T& output) {
	return [&](T result) {
		output = result;
	};
}

template<class T, class Context>
auto assignTo(T& output, stx::weak<Context> if_this_still_exists) {
	return [&output, if_this_still_exists](T result) {
		if(auto tmp = if_this_still_exists.lock()) {
			output = result;
		}
	};
}

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
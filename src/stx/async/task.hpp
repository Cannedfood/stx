#pragma once

#include "../async.hpp"

// TODO

namespace stx {

template<class Callback>
class task_builder {
	stx::executor& m_executor;
	Callback       m_callback;

	template<class Callback2>
	auto then(Callback2&& cb) {
		return task_builder {
			m_executor,
			[callback = std::move(cb)]() {

			}
		};
	}

	template<class Callback2>
	auto fail(Callback2&& cb) {

	}
};

template<class Callback>
auto defer(Callback&& cb) {

}

} // namespace stx
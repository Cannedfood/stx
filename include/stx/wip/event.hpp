#pragma once

#define STX_WIP

#include "../handle.hpp"

#include <vector>
#include <list>

#include "event.inl"

namespace stx {

// == Event policies =========================================================
namespace result_policy {

/*
using ignore = detail::result_policy::ignore_struct;
using collect = detail::result_policy::collect_struct;
using first_true = detail::result_policy::first_true_struct;
using first_false = detail::result_policy::first_false_struct;
*/

} // namespace event_policy


// == event<fn, policy> ======================================================
template<typename FnSig, typename Policy = void>
class event {
	using observer = detail::observer_interface_of<FnSig>;

	observer* m_observers;
public:
	event() : m_observers(nullptr) {}

	~event() {
		clear_observers();
	}

	void add_observer(observer* o) {
		o->attach_to(&m_observers);
	}

	template<typename Fn>
	void add_observer(Fn&& fn) {
		add_observer(detail::create_new_observer<FnSig>(std::forward<Fn>(fn)));
	}

	void clear_observers() {
		while(m_observers) {
			m_observers->detach();
		}
	}

	template<typename... ARGS>
	void trigger(ARGS... args) {
		observer* o = m_observers;
		while(o) {
			o->on_event(std::forward<ARGS>(args)...);
			o = o->next();
		}
	}
};

} // namespace stx

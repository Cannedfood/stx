#pragma once

#define STX_WIP

#include <vector>
#include <list>

#include "../handle.hpp"

#include "event.inl"

namespace stx {

// == Event policies =========================================================
namespace result_policy {

using ignore = detail::result_policy::ignore_struct;
using first_true = detail::result_policy::first_true_struct;
using first_false = detail::result_policy::first_false_struct;
using event_object = detail::result_policy::event_object_policy_struct;

template<typename T>
using default_policy = ignore;

} // namespace event_policy


// == event<fn, policy> ======================================================
template<typename FnSig, typename Policy = result_policy::default_policy<FnSig>>
class event {
	using observer = detail::observer_interface_of<FnSig>;

	observer* m_observers;
public:
	event() : m_observers(nullptr) {}

	~event() {
		clear_observers();
	}

	observer* add_observer(observer* o) {
		o->attach_to(&m_observers);
		return o;
	}

	template<typename Fn>
	observer* add_observer(Fn&& fn) {
		return add_observer(detail::create_new_observer<FnSig>(std::forward<Fn>(fn)));
	}

	void clear_observers() {
		while(m_observers) {
			m_observers->detach();
		}
	}

	template<typename... ARGS>
	auto trigger(ARGS&&... args) {
		return Policy()(m_observers, std::forward<ARGS>(args)...);
	}
};

struct event_object {
private:
	mutable bool m_handled = false;

public:
	void mark_handled() const noexcept { m_handled = true; }
	bool handled()      const noexcept { return m_handled; }
	bool unhandled()    const noexcept { return !handled(); }
};

template<typename T>
using object_event = event<void(T const&), stx::result_policy::event_object>;

} // namespace stx

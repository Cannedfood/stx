#pragma once

#include <vector>

namespace stx {

namespace detail {

namespace event_policy {

template<typename T>
struct collector_policy_struct : std::vector<T> {
	template<typename Container, typename... ARGS>
	void operator()(Container&& callbacks, ARGS&&... args) {
		std::vector<T>::reserve(callbacks.size());
		for(auto& c : callbacks)
			push_back(c(std::forward<ARGS>(args)...));
	}
};

template<typename T>
struct default_policy_struct;

template<typename T, typename... ARGS>
struct default_policy_struct<T(ARGS...)> {
	using type = collector_policy_struct<T>();
};

} // namespace event_policy

} // namespace detail

namespace event_policy {

template<typename T>
using default_policy = detail::event_policy::default_policy_struct<T>;

} // namespace event_policy

template <typename Fn, typename Policy = event_policy::default_policy<Fn>>
class event {
public:
	event();
	~event() noexcept;
};

} // namespace stx

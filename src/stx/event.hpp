// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#ifndef STX_EVENT_HPP_INCLUDED
#define STX_EVENT_HPP_INCLUDED

#pragma once

#include "list.hpp"

#include <utility>

namespace stx {

template<class... Args>
class listener;
template<class... Args>
class event;

template<class... Args>
class listener : private list_element<listener<Args...>> {
public:
	using node_t     = list_element<listener<Args...>>;
	using listener_t = listener<Args...>;
	using event_t    = event<Args...>;

	using node_t::remove;

	virtual void onEventCleared() {}
	virtual void on(Args... args) = 0;

	operator listener_t*() noexcept { return this; }

private:
	friend list_element<listener_t>;
	friend list<listener_t>;
	friend event_t;
};

template<class... ListenerTypes>
class multi_listener : public ListenerTypes... {
public:
	void removeListeners() { (ListenerTypes::remove(), ...); }
};

template<class C, bool autodelete = true, class... Args>
class callback_listener final : public listener<Args...> {
	C m_callback;
public:
	template<class... CArgs>
	callback_listener(CArgs&&... cargs) :
		m_callback(std::forward<CArgs>(cargs)...)
	{}

	void onEventCleared() { if constexpr(autodelete) delete this; }
	void on(Args... args) { m_callback(args...); }
};

template<class... Args>
class event : public list<listener<Args...>> {
	using list_t = list<listener<Args...>>;
public:
	using event_t = event<Args...>;
	using listener_t = listener<Args...>;
	template<class Callback, bool autodelete>
	using callback_listener_t = callback_listener<std::remove_reference_t<Callback>, autodelete, Args...>;

	constexpr event() noexcept;
	constexpr event(event_t&& other) = default;
	constexpr event(event_t const& other) = delete;
	~event();

	using list_t::add;
	template<class C> std::enable_if_t<std::is_invocable_v<C, Args...>,
	callback_listener_t<C, true>*> add(C&& c) {
		auto listener = new callback_listener_t<C, true>(std::forward<C>(c));
		add(listener);
		return listener;
	}

	event_t& operator=(event_t const& other) = default;
	event_t& operator=(event_t&& other) = default;

	constexpr
	void clear() noexcept;

	template<class Pred = bool(*)(Args...)>
	void send(Args... args, Pred&& pred = [](Args...) { return true; }) {
		for(auto& l : *this) {
			if(!pred(args...)) break;
			l.on(args...);
		}
	}

	template<class Pred = bool(*)(Args...)>
	void operator()(Args... args, Pred&& pred = [](Args...) { return true; }) {
		send(args..., std::forward<Pred>(pred));
	}
};

} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx {

template<class... Args> constexpr
event<Args...>::event() noexcept {}
template<class... Args>
event<Args...>::~event() { clear(); }

template<class... Args> constexpr
void event<Args...>::clear() noexcept {
	while(!list_t::empty()) {
		listener_t* l = list_t::begin();
		l->remove();
		l->onEventCleared();
	}
}

} // namespace stx

#endif // header guard STX_EVENT_HPP_INCLUDED

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

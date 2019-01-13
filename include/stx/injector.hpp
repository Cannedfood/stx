#pragma once

#include <unordered_map>
#include <array>
#include <functional>

#include "shared.hpp"

namespace stx {

class injector {
public:
	using entry_t   = shared<void>;
	using factory_t = std::function<entry_t(injector&)>;
	using factory_listener_t = std::function<void(entry_t const&, injector&)>;

	entry_t get(std::type_info const& info, size_t quirk);
	void entry   (entry_t   e, std::type_info const& info, size_t quirk);
	void factory (factory_t f, std::type_info const& info, size_t quirk);
	void listener(factory_listener_t f, std::type_info const& info, size_t quirk);

	template<class T> shared<T> get(size_t quirk = 0);
	template<class T> injector& entry(shared<T>, size_t quirk = 0);
	template<class T> injector& entry(size_t quirk = 0);
	template<class T> shared<T> emplace_entry(size_t quirk = 0);
	template<class T> injector& factory(factory_t f, size_t quirk = 0);
	template<class T> injector& factory(size_t quirk = 0);

	template<class T> operator shared<T>() { return get<T>(); }
	template<class... Tn> void operator()(shared<Tn>&... t) {
		std::tie(t...) = std::tuple{ get<Tn>()... };
	}
public:
	std::unordered_map<size_t, entry_t>   m_entries;
	std::unordered_map<size_t, factory_t> m_factories;
};

} // namespace stx

// =============================================================
// == Inline implementation ====================================
// =============================================================

namespace stx {

template<class T>
shared<T> injector::get(size_t quirk) {
	return get(typeid(T), quirk).cast_static<T>();
}
template<class T>
injector& injector::entry(size_t quirk) {
	entry(stx::make_shared<T>(), typeid(T), quirk); return *this;
}
template<class T>
injector& injector::entry(shared<T> e, size_t quirk) {
	entry(e, typeid(T), quirk); return *this;
}
template<class T>
shared<T> injector::emplace_entry(size_t quirk) {
	auto result = stx::make_shared<T>();
	entry(result, typeid(T), quirk);
	return result;
}
template<class T>
injector& injector::factory(size_t quirk) {
	factory([](auto& injector) { return stx::make_shared<T>(); }, typeid(T), quirk); return *this;
}
template<class T>
injector& injector::factory(factory_t f, size_t quirk) {
	factory(f, typeid(T), quirk); return *this;
}

} // namespace stx

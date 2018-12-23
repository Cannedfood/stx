#pragma once

#include <unordered_map>
#include <array>
#include <functional>

#include "shared_ptr.hpp"

namespace stx {

class injector {
public:
	using entry_t   = std::shared_ptr<void>;
	using factory_t = std::function<entry_t(injector&)>;
	using factory_listener_t = std::function<void(entry_t const&, injector&)>;

	entry_t get(std::type_info const& info, size_t quirk);
	void entry   (entry_t   e, std::type_info const& info, size_t quirk);
	void factory (factory_t f, std::type_info const& info, size_t quirk);
	void listener(factory_listener_t f, std::type_info const& info, size_t quirk);

	template<class T> std::shared_ptr<T> get(size_t quirk = 0);
	template<class T> injector& entry(std::shared_ptr<T>, size_t quirk = 0);
	template<class T> injector& entry(size_t quirk = 0);
	template<class T> injector& factory(factory_t f, size_t quirk = 0);
	template<class T> injector& factory(size_t quirk = 0);

	template<class T> operator std::shared_ptr<T>() { return get<T>(); }
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
std::shared_ptr<T> injector::get(size_t quirk) {
	return std::static_pointer_cast<T>(get(typeid(T), quirk));
}
template<class T>
injector& injector::entry(size_t quirk) {
	entry(std::make_shared<T>(), typeid(T), quirk); return *this;
}
template<class T>
injector& injector::entry(std::shared_ptr<T> e, size_t quirk) {
	entry(e, typeid(T), quirk); return *this;
}
template<class T>
injector& injector::factory(size_t quirk) {
	factory([](auto& injector) { return std::make_shared<T>(); }, typeid(T), quirk); return *this;
}
template<class T>
injector& injector::factory(factory_t f, size_t quirk) {
	factory(f, typeid(T), quirk); return *this;
}

} // namespace stx

#pragma once

#include <unordered_map>
#include <array>
#include <functional>

#include "shared_ptr.hpp"

namespace stx {

class injector {
public:
	using entry   = std::shared_ptr<void>;
	using factory = std::function<entry(injector&)>;
	using factory_listener = std::function<void(entry const&, injector&)>;

	entry request   (std::type_info const& info, size_t quirk);
	void  add       (entry   e, std::type_info const& info, size_t quirk);
	void  addFactory(factory f, std::type_info const& info, size_t quirk);
	void  addFactoryListener(factory_listener f, std::type_info const& info, size_t quirk);

	template<class T> std::shared_ptr<T> request(size_t quirk = 0);
	template<class T> void add(std::shared_ptr<T>, size_t quirk = 0);
	template<class T> void add(size_t quirk = 0);
	template<class T> void addFactory(factory f, size_t quirk = 0);
	template<class T> void addFactory(size_t quirk = 0);

	template<class T>
	operator std::shared_ptr<T>() {
		return std::static_pointer_cast<T>(request(typeid(T), 0));
	}
public:
	std::unordered_map<size_t, entry>   m_entries;
	std::unordered_map<size_t, factory> m_factories;
};

} // namespace stx

// =============================================================
// == Inline implementation ====================================
// =============================================================

namespace stx {

template<class T>
std::shared_ptr<T> injector::request(size_t quirk) {
	return std::static_pointer_cast<T>(request(typeid(T), quirk));
}
template<class T>
void injector::add(size_t quirk) {
	add(std::make_shared<T>(), typeid(T), quirk);
}
template<class T>
void injector::add(std::shared_ptr<T> e, size_t quirk) {
	add(e, typeid(T), quirk);
}
template<class T>
void injector::addFactory(size_t quirk) {
	addFactory([](auto& injector) { return std::make_shared<T>(); }, typeid(T), quirk);
}
template<class T>
void injector::addFactory(factory f, size_t quirk) {
	addFactory(f, typeid(T), quirk);
}

} // namespace stx

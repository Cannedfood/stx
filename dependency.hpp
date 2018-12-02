// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

#pragma once

#include <unordered_map>
#include <typeinfo>

#include "shared_ptr.hpp"

namespace stx {

class dependencies {
	using dependency_id = std::size_t;

	std::unordered_map<dependency_id, shared<void>> m_factories;
public:
	template<class T> class factory;

	template<class T> void singleton(size_t quirk = 0);
	template<class T> void singleton(std::string_view quirk);
	template<class T> void transient(size_t quirk = 0);
	template<class T> void transient(std::string_view quirk);

	template<class T> T get();

	template<class Result, class... Args>
	Result invoke(Result(*pfn)(Args...)) { return pfn(get<Args>()...); }

	template<class Class, class Result, class... Args>
	Result invoke(Class* obj, Result(Class::*pfn)(Args...)) { return (*obj).*pfn(get<Args>()...); }
};

template<class T>
class dependencies::factory {
public:
	virtual shared<T> shared() = 0;
	virtual T         value() = 0;
};

} // namespace stx

// =============================================================
// == Inline implementation ====================================
// =============================================================

namespace stx {

namespace detail {

template<class T>
class singleton_factory final : public dependencies::factory<T> {
public:
	shared<T> shared() override { return nullptr; }
	T         value()  override { throw std::runtime_error("Cannot get singleton by value"); }
};

template<class T>
class transient_factory : public dependencies::factory<T> {
public:
	shared<T> shared() override { return make_shared<T>(); }
	T         value()  override { return {}; }
};

size_t make_dependency_id(std::type_info& type, size_t quirk) {
	return (type.hash_code() + quirk) ^ quirk;
}

} // namespace detail

template<class T>
dependencies::factory& dependencies::singleton(size_t quirk) {
	auto factory = make_shared<singleton_factory<T>>();
	m_factories[make_dependency_id(typeid(T), quirk)] = factory;
	return factory;
}
template<class T>
dependencies::factory& dependencies::transient(size_t quirk) {
	auto factory = make_shared<singleton_factory<T>>();
	m_factories[make_dependency_id(typeid(T), quirk)] = factory;
	return factory;
}

template<class T>
T dependencies::get() {
	return {}; // TODO
}



} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

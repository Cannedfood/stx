// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!! BEWARE: This will break when linking static libraries !!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! To be more precise: If you register a class/get_specialization !
!  etc. in a static library and link it, the linker will remove  !
!  the symbols and the classes will never be registered.         !
*/

/*
Usage:

class base0 {};
class base1 {};

class derived : base0, base1 {};

stx::classes::register_specialization<base0, derived>;
stx::classes::register_specialization<base1, derived>;
*/

#ifndef STX_CLASS_REGISTRY_HPP_INCLUDED
#define STX_CLASS_REGISTRY_HPP_INCLUDED

#pragma once

#include <typeinfo>
#include <set>
#include <tuple>
#include <iosfwd>

#include "../list.hpp"
#include "../type.hpp"

namespace stx {

class classes {
public:
	class entry;
	class base_class;
	class derived_class;

	template<class Base, class Derived>
	class register_implementation;

	template<class Class>
	class register_class;

	static void print(std::ostream& s);

	template<class T> static
	entry& get() { return register_class<T>::instance(); }

	static list<entry>& entries() noexcept;
};

class classes::entry : private list_element<entry> {
	friend list_element_t;
	std::type_info const* m_type;
	list<derived_class> m_derived_classes;
	list<base_class>    m_base_classes;

	friend class derived_class;
	friend class base_class;
protected:
	entry(std::type_info const& type) :
		m_type(&type)
	{
		classes::entries().add(this);
	}
public:
	std::string name() { return stx::demangle(m_type->name()); }
	list<derived_class> const& implementations() { return m_derived_classes; }
	list<base_class>    const& implements() { return m_base_classes; }
};

class classes::base_class : private list_element<base_class> {
	friend list_element_t;
	entry* m_base_class;
protected:
	base_class(entry& base, entry& derived) :
		m_base_class(&base)
	{
		derived.m_base_classes.add(this);
	}
public:
	entry* get_base()         const noexcept { return m_base_class; }
	entry*       operator->()       noexcept { return m_base_class; }
	entry const* operator->() const noexcept { return m_base_class; }
	entry&       operator*()        noexcept { return *m_base_class; }
	entry const& operator*()  const noexcept { return *m_base_class; }
	operator entry&      ()         noexcept { return *m_base_class; }
	operator entry const&()   const noexcept { return *m_base_class; }
};

class classes::derived_class : public base_class, private list_element<derived_class> {
	friend struct list_element<derived_class>;
	entry* m_derived_class;
protected:
	derived_class(entry& base, entry& derived) :
		base_class(base, derived),
		m_derived_class(&derived)
	{
		base.m_derived_classes.add(this);
	}
public:
	entry* get_specialization() const noexcept { return m_derived_class; }
	entry*       operator->()       noexcept { return m_derived_class; }
	entry const* operator->() const noexcept { return m_derived_class; }
	entry&       operator*()       noexcept { return *m_derived_class; }
	entry const& operator*() const noexcept { return *m_derived_class; }
	operator entry&      ()       noexcept { return *m_derived_class; }
	operator entry const&() const noexcept { return *m_derived_class; }
};

template<class Class>
class classes::register_class : public classes::entry {
protected:
	register_class() :
		classes::entry(typeid(Class))
	{}
public:
	static classes::register_class<Class>& instance() {
		static classes::register_class<Class> v_instance;
		return v_instance;
	}
private:
	static classes::register_class<Class>& g_instance;
};
template<class Class>
classes::register_class<Class>& classes::register_class<Class>::g_instance = instance();

template<class Base, class Derived>
class classes::register_implementation : public classes::derived_class {
protected:
	register_implementation() : derived_class(
		classes::register_class<Base>::instance(), classes::register_class<Derived>::instance())
	{}

	static register_implementation<Base, Derived>& instance() {
		static register_implementation<Base, Derived> v_instance;
		return v_instance;
	}
private:
	static register_implementation<Base, Derived>& g_instance;
};
template<class Base, class Derived>
classes::register_implementation<Base, Derived>& classes::register_implementation<Base, Derived>::g_instance = instance();

} // namespace stx

#endif // header guard STX_CLASS_REGISTRY_HPP_INCLUDED

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

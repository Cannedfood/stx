#ifndef STX_SYSTEM_HPP_INCLUDED
#define STX_SYSTEM_HPP_INCLUDED

#pragma once

#include "graph.hpp"

namespace stx {

template<class Manager, class System>
class basic_system_manager;

template<class Base, class Manager>
class basic_system : private stx::list_element<Base> {
	friend class basic_system_manager<Manager, Base>;
protected:
	basic_system();
public:
	virtual ~basic_system();
};

template<class Manager, class System>
class basic_system_manager {

};

} // namespace stx

#endif // header guard STX_SYSTEM_HPP_INCLUDED

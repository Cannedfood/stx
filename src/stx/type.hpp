#ifndef STX_TYPE_INCLUDED
#define STX_TYPE_INCLUDED

#include <string>
#include <typeinfo>

#pragma once

namespace stx {

std::string demangle(const char* n);

template<class T>
std::string nameof(T const* t) { return demangle(typeid(*t).name()); }
template<class T>
std::string nameof(T const& t) { return demangle(typeid(t).name()); }

} // namespace stx

#endif // header guard STX_TYPE_INCLUDED

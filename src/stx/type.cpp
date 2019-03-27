#include "type.hpp"

#ifdef __GNUC__

#include <cstdlib>
#include <memory>
#include <cxxabi.h>

std::string stx::demangle(const char* name) {
    int status = -1;
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };
    return (status == 0) ? res.get() : name;
}

#else

std::string stx::demangle(const char* c) {
	return c;
}

#endif

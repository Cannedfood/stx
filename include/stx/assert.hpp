#pragma once

#include "platform.hpp"

#include <stdexcept>
#include <string>


#define xfatal(MSG)                                                                                \
	do {                                                                                           \
		throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " in " + \
		                         STX_FUNCTION + " : " + MSG);                                      \
	} while(false)

#ifdef STX_DEBUG_TOOLS

#define xassert(TEST)                  \
	do {                               \
		if(!(TEST)) { xfatal(#TEST); } \
	} while(false)

#define xassertmsg(TEST, MSG)                \
	do {                                     \
		if(!(TEST)) { xfatal(#TEST + MSG); } \
	} while(false)

#else // defined(STX_DEBUG_TOOLS)

#define xassert(TEST) \
	do { TEST; } while(false)
#define xassertmsg(TEST, MSG) \
	do { TEST; } while(false)

#endif // defined(STX_DEBUG_TOOLS)

#if STX_DEBUG_TOOLS > 0

#include "logging.hpp"

namespace stx {

/// A class used for debugging which checks whether a returned value was handled
template <typename T>
class important {
	T            m_value;
	mutable bool m_handled;

public:
	constexpr important(bool val) : m_value(val), m_handled(false) {}

	~important() noexcept {
		if(!m_handled) {
			stx::warn("Unhandled important value! (important function result not handled)");
		}
	}

	constexpr inline operator T() const noexcept {
		m_handled = true;
		return m_value;
	}
};

} // namespace stx

#else // STX_DEBUG_TOOLS > 0

namespace stx {

template <typename T>
using important = bool;

} // namespace stx

#endif // STX_DEBUG_TOOLS > 0

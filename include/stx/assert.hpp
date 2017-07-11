#pragma once

#include "platform.hpp"

#include <exception>
#include <string>


#define xfatal(MSG)                                                     \
	do {                                                                \
		fputs((std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
		       " in " + STX_FUNCTION + " : " + MSG)                     \
		          .c_str(),                                             \
		      stderr);                                                  \
		std::terminate();                                               \
	} while(false)

#define xfail(MSG)                                                   \
	do {                                                             \
		throw std::runtime_error(std::string(__FILE__) + ":" +       \
		                         std::to_string(__LINE__) + " in " + \
		                         STX_FUNCTION + " : " + MSG);        \
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

// These are intentionally not defined when the debug tools are off:
// they are only for debugging and should NEVER be in production code build
#define xhere() printf("%s:%i:\n", STX_FUNCTION, __LINE__)
#define xhere_class() printf("%s:%i: (%p)\n", STX_FUNCTION, __LINE__, this)

#else // defined(STX_DEBUG_TOOLS)

#define xassert(TEST) \
	do {              \
		if(TEST) {}   \
	} while(false) // The if is necessary to supress warnings
#define xassertmsg(TEST, MSG) \
	do {                      \
		if(TEST) {}           \
	} while(false)

#endif // defined(STX_DEBUG_TOOLS)

#if STX_DEBUG_TOOLS > 0

#include "logging.hpp"

namespace stx {

/// A class used for debugging which checks whether a returned value was
/// handled
template <typename T>
class important {
	T            m_value;
	mutable bool m_handled;

public:
	template <typename Tx>
	constexpr important(Tx&& val) : m_value(val), m_handled(false) {}

	~important() noexcept {
		if(!m_handled) {
			stx::warn(
			    "Unhandled important value! (important function result not "
			    "handled)");
		}
	}

	constexpr inline operator T() const noexcept {
		m_handled = true;
		return std::move(m_value);
	}
};

/// A class used for debugging which checks whether a returned value was
/// handled
template <typename T>
class asserted {
	T            m_value;
	mutable bool m_handled;

public:
	template <typename Tx>
	constexpr asserted(Tx&& val) : m_value(val), m_handled(false) {}

	~asserted() {
		if(!m_handled) {
			if(!m_value) {
				xfatal(
				    "Asserted return value was not handled and evaluates "
				    "to "
				    "false");
			} else {
				stx::warn(
				    "Unhandled important value! (important function result "
				    "not "
				    "handled)");
			}
		}
	}

	constexpr inline operator T() const noexcept {
		m_handled = true;
		return std::move(m_value);
	}
};

} // namespace stx

#define STX_RESULT_HANDLED(X) \
	{                         \
		if(X) {}              \
	}

#else // STX_DEBUG_TOOLS > 0

namespace stx {

template <typename T>
using important = T;

template <typename T>
using asserted = T;

} // namespace stx

#define STX_RESULT_HANDLED(X) \
	{ X; }

#endif // STX_DEBUG_TOOLS > 0

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

	~important() {
		if(!m_handled) {
#ifdef STX_FATAL_IMPORTANT_VALUES
			throw std::runtime_error(
			    "Unhandled important value! (important function result not handled)");
#else
			stx::warn(
			    "Unhandled important value! (important function result not "
			    "handled)");
#endif
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
			}
			else {
				#ifdef STX_FATAL_IMPORTANT_VALUES
							throw std::runtime_error(
							    "Unhandled important value! (important function result not handled, but it was not an error)");
				#else
							stx::warn(
							    "Unhandled important value! (important function result "
							    "not "
							    "handled, but it was not an error)");
				#endif
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

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

#include <cassert>

#define xassert(TEST) \
	do { \
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

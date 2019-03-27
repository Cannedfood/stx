#ifndef STX_API_HPP_INCLUDED
#define STX_API_HPP_INCLUDED

#pragma once

#define STX_STATIC static
#ifdef _WIN32
	#define STX_LOCAL
  #ifdef __GNUC__
    #define STX_EXPORT __attribute__ ((dllexport))
		#define STX_IMPORT __attribute__ ((dllimport))
  #else
    #define STX_EXPORT __declspec(dllexport)
		#define STX_IMPORT __declspec(dllimport)
  #endif
#elif __GNUC__
	#define __attribute__ ((visibility ("hidden")))
	#define STX_EXPORT __attribute__ ((visibility ("default")))
	#define STX_IMPORT __attribute__ ((visibility ("default")))
#else
	#error "api.hpp not implemented on this platform. Change it. It should be trivial"
#endif

#endif // header guard STX_API_HPP_INCLUDED

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
	#define STX_LOCAL __attribute__ ((visibility ("hidden")))
	#define STX_EXPORT __attribute__ ((visibility ("default")))
	#define STX_IMPORT __attribute__ ((visibility ("default")))
#else
	#error "api.hpp not implemented on this platform. Change it. It should be trivial"
#endif

/// Mark all STX_PUBLIC symbols after STX_EXPORT_AFTER_THIS as STX_EXPORT
#ifdef STX_EXPORT_AFTER_THIS
	#define STX_PUBLIC STX_EXPORT
#else
	#define STX_PUBLIC STX_IMPORT
#endif

#endif // header guard STX_API_HPP_INCLUDED

/// @file:
/// Definitions: <br/>
/// OS:
/// <ul>
///    <li> STX_OS_LINUX </li>
///    <li> STX_OS_MACOSX </li>
///    <li> STX_OS_WINDOWS </li>
///    <li> STX_OS_UNIX (enabled on linux & macosx) </li>
///    <li> (Sorry solaris & BSD) </li>
/// </ul>
/// Compiler:
/// <ul>
///    <li> STX_COMPILER_GCC (also defined for clang, because they are very much alike) </li>
///    <li> STX_COMPILER_CLANG (Defined together with STX_COMPILER_GCC, because clang is very close to it) </li>
///    <li> STX_COMPILER_MSVC </li>
/// </ul>
/// Architecture:
/// <ul>
///    <li> STX_ARCHITECTURE_x64 </li>
///    <li> STX_ARCHITECTURE_x86 </li>
///    <li> STX_ARCHITECTURE_ARM </li>
/// </ul>
/// Endianness:
/// <ul>
///    <li> STX_BYTEORDER_BIG_ENDIAN </li>
///    <li> STX_BYTEORDER_LITTLE_ENDIAN </li>
/// </ul>

#pragma once // Supported by almost all compilers, so let's just always do this

/* Compiler & OS */

#ifdef __GNUC__ // Detecting Compiler (GCC-like)
#	if defined(__APPLE__) && defined(__MACH__) // Detecting OS (macosx)
#		define STX_OS_MACOSX 1
#       define STX_OS_UNIX 1
#	elif defined(__gnu_linux__) // Detecting OS (some linux)
#		define STX_OS_LINUX  1
#       define STX_OS_UNIX 1
#	else
#		error "Unknown OS, is it very similar to Linux? yes -> Just copy the defines around STX_OS_LINUX. no -> you're screwed."
#	endif // Detecting OS
#	define STX_COMPILER_GCC 1 // (or clang)
#   ifdef __clang__
#	   define STX_COMPILER_CLANG
#   endif // Detecting clang
#elif defined(_WIN32) // Detecting Compiler (Windows)
#	define STX_COMPILER_MSVC 1
#   define STX_OS_WINDOWS 1
#else // Detecting Compiler
#	error "Unknown compiler or OS"
#endif // Detecting Compiler


/* Architecture */

#if   defined(__x86_64) || defined(_M_X64)
#	define STX_ARCHITECTURE_x64 1
#elif defined(__i386) || defined(__M_IX86)
#	define STX_ARCHITECTURE_x86 1
#elif defined(__arm__) || defined(_M_ARM)
#   define STX_ARCHITECTURE_ARM 1
#	error "ARM architecture not currently supported"
#else
#   error "Unknown architecture (or couldn't detect it)"
#endif

/* Platform dependent defines */

#if STX_COMPILER_GCC
#	define STX_CONSTEXPR constexpr
#	define STX_FORCEINLINE __attribute__((always_inline))
#	define STX_API
#	define STX_FUNCTION __PRETTY_FUNCTION__
#elif STX_COMPILER_MSVC
#	define STX_CONSTEXPR
#	define STX_FORCEINLINE __forceinline
#   if !defined(STX_DLL)
#		define STX_API
#	elif defined(STX_COMPILE_DLL)
#		define STX_API __declspec(dllexport)
#	else
#		define STX_API __declspec(dllimport)
#	endif
#	define STX_FUNCTION __FUNCSIG__
#else
#	error "Unknown compiler"
#endif



/* Various */

/// Shortcut for STX_CONSTEXPR STX_FORCEINLINE
#define STX_CONSTEXPR_FORCEINLINE STX_CONSTEXPR STX_FORCEINLINE

/* Endianness */

#if defined(__BYTE_ORDER__)
#	if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#		define STX_BYTEORDER_BIG_ENDIAN 1
#	elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#		define STX_BYTEORDER_LITTLE_ENDIAN 1
#	endif
#else
#	error "Couldn't detect byte order"
#endif

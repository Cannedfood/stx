// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "shared_lib.hpp"

#include <utility>

#ifdef __GNUC__ // Detecting Compiler (GCC-like)
#	if defined(__APPLE__) && defined(__MACH__) // Detecting OS (macosx)
#		define STX_OS_MACOSX 1
#       define STX_OS_UNIX 1
#	elif defined(__gnu_linux__) // Detecting OS (some linux)
#		define STX_OS_LINUX  1
#       define STX_OS_UNIX 1
#	elif defined(__MINGW32__)
#		define STX_OS_UNIX 1
#	else
#		error "Unknown OS, is it very similar to Linux? yes -> Just copy the defines around STX_OS_LINUX. no -> you're screwed."
#	endif // Detecting OS
#	define STX_COMPILER_GCC 1 // (or clang)
#   ifdef __clang__
#	   define STX_COMPILER_CLANG 1
#	elif defined(__MINGW32__)
#		define STX_COMPILER_MINGW 1
#   endif // Detecting clang
#elif defined(_WIN32) // Detecting Compiler (Windows)
#	define STX_COMPILER_MSVC 1
#   define STX_OS_WINDOWS 1
#else // Detecting Compiler
#	error "Unknown compiler or OS"
#endif // Detecting Compiler

namespace stx {

shared_lib::shared_lib() : m_handle(nullptr) {}

shared_lib::shared_lib(const char* path) :
	shared_lib()
{
	open(path);
}

shared_lib::~shared_lib() noexcept {
	close();
}

shared_lib::shared_lib(shared_lib&& other) noexcept :
	m_handle(other.m_handle)
{
	other.m_handle = nullptr;
}

shared_lib& shared_lib::operator=(shared_lib&& other) noexcept {
	std::swap(m_handle, other.m_handle);
	other.close();
	return *this;
}

} // namespace stx

// == Windows implementation ==============================================================
#if STX_OS_WINDOWS

#include <Windows.h>

#include <string>

namespace stx {

bool shared_lib::open(const char* libname, unsigned flags) noexcept {
	m_handle = LoadLibrary((std::string(filename) + ".dll").c_str());
	return is_open();
}

void shared_lib::close() noexcept {
	if(m_handle) {
		FreeLibrary(m_handle);
		m_handle = nullptr;
	}
}

void* shared_lib::getp(const char* symbol) const noexcept { return GetProcAdress(m_handle, symbol); }

unsigned shared_lib::supported_flags() noexcept { return 0; }

} // namespace stx

// == Macosx implementation
#elif STX_OS_MACOSX
#error "Not implemented on macos"

// == Linux/Unix implementation ==============================================================
#elif STX_OS_LINUX

#include <dlfcn.h>

#include <stdio.h>

#include <string>

namespace stx {

bool shared_lib::open(const char* path, unsigned flags) noexcept {
	std::string name = path;
	if(!(flags & no_prefix)) {
		size_t last_fwd_slash = name.rfind('/');
		if(last_fwd_slash == std::string::npos)
			name = "lib" + name;
		else
			name.insert(last_fwd_slash + 1, "lib");
	}

	if(!(flags & no_postfix)) {
		name += ".so";
	}

	int mode = 0;

	if(flags & lazy)
		mode |= RTLD_LAZY;
	else
		mode |= RTLD_NOW;

	if(flags & nodelete) mode |= RTLD_NODELETE;
	if(flags & global) mode |= RTLD_GLOBAL;
	if(flags & resident_only) mode |= RTLD_NOLOAD;

	m_handle = dlopen(name.c_str(), mode);

	if(!is_open()) {
		fprintf(stderr, "Failed opening shared lib: %s\n", dlerror());
	}

	return is_open();
}

void shared_lib::close() noexcept {
	if(m_handle) {
		if(dlclose(m_handle)) {
			fprintf(stderr, "Failed closing lib %p: %s\n", m_handle, dlerror());
		}
		m_handle = nullptr;
	}
}

void* shared_lib::getp(const char* symbol) const noexcept { return dlsym(m_handle, symbol); }

unsigned shared_lib::supported_flags() noexcept {
	return lazy | nodelete | global | resident_only | custom_symbols;
}

} // namespace stx

// == Macosx & Others ==============================================================
#else
#error Not implemented for this platform
#endif

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

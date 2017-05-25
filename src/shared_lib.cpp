#include "../include/stx/shared_lib.hpp"

#include "../include/stx/platform.hpp"

#include <utility>

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

shared_lib::shared_lib(shared_lib&& other) :
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

void* shared_lib::getp(const char* symbol) noexcept { return GetProcAdress(m_handle, symbol); }

unsigned shared_lib::supported_flags() noexcept { return 0; }

} // namespace stx

// == Linux implementation ==============================================================
#elif STX_OS_LINUX

#include <dlfcn.h>
#include <errno.h>  // for errno
#include <string.h> // for strerror

#include <stdio.h>

#include <string>

namespace stx {

bool shared_lib::open(const char* path, unsigned flags) noexcept {
	std::string name = std::string("lib") + path + ".so";

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
		fprintf(stderr, "Failed shared lib '%s': %s", name.c_str(), ::strerror(errno));
	}

	return is_open();
}

void shared_lib::close() noexcept {
	if(m_handle) {
		if(!dlclose(m_handle)) {
			fprintf(stderr, "Failed closing lib %p: %s", m_handle, ::strerror(errno));
		}
		m_handle = nullptr;
	}
}

void* shared_lib::getp(const char* symbol) noexcept { return dlsym(m_handle, symbol); }

unsigned shared_lib::supported_flags() noexcept {
	return lazy | nodelete | global | resident_only | custom_symbols;
}

} // namespace stx

// == Macosx & Others ==============================================================
#else
#error Not implemented for this platform
#endif

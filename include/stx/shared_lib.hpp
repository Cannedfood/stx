// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

namespace stx {

/// A shared library
class shared_lib {
	void* m_handle;
public:
	enum flags {
		lazy            = 1, //<! Perform lazy binding.  Resolve symbols only as the code that references them is executed.
		nodelete        = 2, //<! Do not actually close the library later. Therefore static objects will not be recreated when reloading.
		global          = 4, //<! The symbols defined by this shared object will be made available for symbol resolution of subsequently loaded shared objects.
		resident_only   = 8, //<! Library loading should fail if the library isn't already loaded.
		custom_symbols  = 16 //<! Prefer the libraries own symbols over already loaded ones
	};

	// -- Constructors --------------------------------------------------------------
	shared_lib();
	shared_lib(const char* path);
	~shared_lib() noexcept;

	// -- Move & Copy --------------------------------------------------------------
	shared_lib(shared_lib&&) noexcept;
	shared_lib& operator=(shared_lib&&) noexcept;

	shared_lib(shared_lib const&)            = delete;
	shared_lib& operator=(shared_lib const&) = delete;

	// -- Core functionality --------------------------------------------------------------

	/// Open library with the name name. Automatically reformats to lib*.so (unix) or *.dll (windows)
	bool open(const char* name, unsigned flags = 0) noexcept;
	/// Close/unload library
	void close() noexcept;
	/// Get the pointer to the symbol
	void* getp(const char* symbol) const noexcept;

	/// Get the pointer to a symbol and cast it to the type T
	template<typename T> inline
	T* get(const char* symbol) const noexcept { return ((T*) getp(symbol)); }

	/// Load a symbol to a variable. Returns whether the symbol was found.
	template<typename T>
	bool get(T& to, const char* symbol) noexcept {
		if(void* p = getp(symbol)) {
			to = (T) p;
			return true;
		}
		else return false;
	}

	/// Whether the library is open (can also be used to check whether the loading was successful)
	inline bool is_open() const noexcept { return m_handle != nullptr; }

	/// Returns is_open(). @see is_open()
	inline operator bool() const noexcept { return is_open(); }

	// -- Platform dependent --------------------------------------------------------------
	/// Returns the operating system handle of the library
	inline void*    native_handle() noexcept { return m_handle; }
	/// Returns the flags for loading supported by the OS
	static unsigned supported_flags() noexcept;
};

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <type_traits>
#include <cstdlib>
#include <cstdint>

namespace serial {

template<class Derived>
class basic_serial_out {
private:
	uint64_t m_mode;
	Derived& derived() { return *(Derived*) this; }
protected:
	using serial_t = basic_serial_out<Derived>;
public:
	uint64_t mode() const noexcept { return m_mode; }
	Derived& mode(uint64_t m) noexcept { m_mode = m; return derived(); }

	template<class T>
	void write(T* t, size_t n) {
		for(size_t i = 0; i < n; i++) {
			write(t[i]);
		}
	}

	template<class T, size_t n>
	void write(T (&t)[n]) {
		for(auto& tt : t)
			derived().write(tt);
	}

	template<class T> std::enable_if_t<std::is_pod_v<T>,
	Derived&> write(T& var) {
		return derived();
	}
	template<class T>
	Derived& write(T& var) {
		var.save(derived());
		return derived();
	}

	template<class T>
	Derived& operator()(T&& t) {
		derived().write(t);
		return derived();
	}
};

template<class Derived>
class basic_serial_in {
private:
	Derived* derived() { return (Derived*) this; }
public:

};

} // namespace serial

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

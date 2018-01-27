// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <iosfwd>

#include "serial_base.hpp"

namespace serial {

class binary_blobstream {
	uint8_t* data;
	uint8_t* end;
	uint8_t* current;
public:
	size_t read(size_t n, void* to);
};

template<class Stream = std::istream&>
class binary_in : public basic_serial_in<binary_in<Stream>> {
	Stream mStream;
public:
	template<class... StreamArgs>
	binary_in(StreamArgs&&... args) :
		mStream(std::forward<StreamArgs>(args)...)
	{}
};

template<class Stream = std::ostream&>
class binary_out : public basic_serial_out<binary_out<Stream>> {
	Stream mStream;
public:
	template<class... StreamArgs>
	binary_out(StreamArgs&&... args) :
		mStream(std::forward<StreamArgs>(args)...)
	{}

};

} // namespace serial

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

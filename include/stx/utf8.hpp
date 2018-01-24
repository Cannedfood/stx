// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#pragma once

#include <cinttypes>
#include <stdexcept>

namespace stx {

unsigned utf32to8(uint32_t codepoint, char* out, char** end = nullptr);
uint32_t utf8to32(const char* in, const char** out = nullptr);

} // namespace stx

// =============================================================
// == Implementation =============================================
// =============================================================

namespace stx {

namespace detail {

enum utf8_constants : uint32_t {
	utf8_1byte_max = 0x00007F,
	utf8_2byte_max = 0x0007FF,
	utf8_3byte_max = 0x00FFFF,
	utf8_4byte_max = 0x10FFFF,

	utf8_2byte_mask = 0x1F,
	utf8_3byte_mask = 0x0F,
	utf8_4byte_mask = 0x07,
	utf8_cbyte_mask = 0x3F,

	utf8_2byte_pfx = 0xC0,
	utf8_3byte_pfx = 0xE0,
	utf8_4byte_pfx = 0xF0,
	utf8_cbyte_pfx = 0x80
};

constexpr static
unsigned utf8_num_bytes[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,

	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

} // namespace detail

unsigned utf32to8(uint32_t codepoint, char* out) {
	using namespace detail;

	if(codepoint <= utf8_1byte_max) {
		out[0] = codepoint;
		return 1;
	}
	else if(codepoint <= utf8_2byte_max) {
		out[0] = ((codepoint >> 6) & utf8_2byte_mask) | utf8_2byte_pfx;
		out[1] = ((codepoint >> 0) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		return 2;
	}
	else if(codepoint <= utf8_3byte_max) {
		out[0] = ((codepoint >> 12) & utf8_3byte_mask) | utf8_3byte_pfx;
		out[1] = ((codepoint >> 6 ) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		out[2] = ((codepoint >> 0 ) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		return 3;
	}
	else if(codepoint <= utf8_4byte_max) {
		out[0] = ((codepoint >> 18) & utf8_4byte_mask) | utf8_4byte_pfx;
		out[1] = ((codepoint >> 12) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		out[2] = ((codepoint >> 6 ) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		out[3] = ((codepoint >> 0 ) & utf8_cbyte_mask) | utf8_cbyte_pfx;
		return 4;
	}
	else {
		throw std::runtime_error("Codepoint too large!");
	}

	throw std::runtime_error("Not yet implemented");
}

unsigned utf8to32(const char* in, uint32_t& out) {
	using namespace detail;

	unsigned num_bytes = utf8_num_bytes[(unsigned char) *in];

	switch (num_bytes) {
		case 1: return (uint32_t) *in;
		case 2: {
			out =
				(uint32_t(in[0]) & utf8_2byte_mask) << 6 |
				(uint32_t(in[1]) & utf8_cbyte_mask) << 0;
			} break;
		case 3: {
			out =
				(uint32_t(in[0]) & utf8_3byte_mask) << 12 |
				(uint32_t(in[1]) & utf8_cbyte_mask) << 6  |
				(uint32_t(in[2]) & utf8_cbyte_mask) << 0;
			} break;
		case 4: {
			out =
				(uint32_t(in[0]) & utf8_4byte_mask) << 18 |
				(uint32_t(in[1]) & utf8_cbyte_mask) << 12 |
				(uint32_t(in[2]) & utf8_cbyte_mask) << 6  |
				(uint32_t(in[3]) & utf8_cbyte_mask) << 0;
			} break;
		default:
				throw std::runtime_error("Not fully implemented");
	}
	return num_bytes;
}

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "../platform.hpp"

#include <iostream>

bool _testResult(const char* file, int line, const char* fn, const char* test, bool value);

#define test_binop(OP, A, B) \
	do { \
		try { \
			if(!_testResult(__FILE__, __LINE__, STX_FUNCTION, #A #OP #B, (A) OP (B))) return; \
		} catch(std::exception& e) { \
			_testResult(__FILE__, __LINE__, STX_FUNCTION, #A #OP #B, false); \
			std::cerr << '\t' << e.what() << std::endl; return; \
		} \
	} while(false)

#define test_eq(A, B) test_binop(==, A, B)
#define test(X) test_eq(bool(X), true)
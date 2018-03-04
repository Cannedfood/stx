// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "test.hpp"

#include <cstddef>
#include <iostream>

#include <mutex>

// Test compile only
#include "../timer.hpp"

int tests = 0;
int fails = 0;

std::mutex lock;

void _testResult(const char* file, int line, const char* fn, const char* test, bool value) {
	static const char* last_fn = nullptr;

	std::lock_guard<std::mutex> guard(lock);
	++tests;
	if(value) {
		if(last_fn != fn) {
			std::cout << file << ": " << fn << "'" << std::endl;
			last_fn = fn;
		}
	}
	else {
		std::cerr << file << ":" << line << ": error in '" << fn << "' test(" << test << ")  " << "FAIL" << std::endl;
		++fails;
	}
}

void test_xgraph();
void test_xenvironment();
void test_xevent();
void test_xstring();
void test_xdatabase();
void test_xclass_registry();

#include <atomic>

int main(int argc, char const** argv) {
	test_xstring();
	test_xgraph();
	test_xenvironment();
	test_xevent();
	test_xdatabase();
	test_xclass_registry();

	std::cout << "tests: "  << tests << std::endl;
	std::cout << "passed: " << tests - fails << std::endl;
	std::cout << "FAILED: " << fails << std::endl;

	return fails == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

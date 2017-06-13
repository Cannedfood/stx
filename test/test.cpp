#include "test.hpp"

#include <cstddef>
#include <iostream>

#include <mutex>

// Test compile only
#include <xtimer>

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


void test_xmemory();
void test_xhash();
void test_xhandle();
void test_xgraph();
void test_xevent();
void test_xsocket();

int main(int argc, char const** argv) {
	test_xmemory();
	test_xhash();
	test_xhandle();
	test_xgraph();
	test_xevent();
	test_xsocket();


	std::cout << "tests: "  << tests << std::endl;
	std::cout << "passed: " << tests - fails << std::endl;
	std::cout << "FAILED: " << fails << std::endl;

	return fails == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

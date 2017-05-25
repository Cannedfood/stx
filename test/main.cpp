#include "test.hpp"

#include <iostream>

// Test compile only
#include <xtimer>

int tests = 0;
int fails = 0;

void _testResult(const char* file, int line, const char* fn, const char* test, bool value) {
	++tests;
	if(value)
		std::cout << file << ":" << line << ": '" << fn << "'" << std::endl;
	else {
		std::cerr << file << ":" << line << ": error in '" << fn << "' test(" << test << ")  " << "FAIL" << std::endl;
		++fails;
	}
}


void test_xmemory();
void test_xsocket();
void test_xhash();

int main(int argc, char const** argv) {
	test_xmemory();
	test_xhash();
	test_xsocket();


	std::cout << "tests: "  << tests << std::endl;
	std::cout << "passed: " << tests - fails << std::endl;
	std::cout << "FAILED: " << fails << std::endl;

	return 0;
}

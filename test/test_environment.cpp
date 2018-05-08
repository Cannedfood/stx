// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "test.hpp"

#include "../environment.hpp"

static
void test_environment_set_unset_get() {
	stx::env::set("foobar", "barfoo", false);
	test(!strcmp(stx::env::get("foobar"), "barfoo"));
	stx::env::set("foobar", "barfoo2000", false);
	test(strcmp(stx::env::get("foobar"), "barfoo2000"));
	stx::env::set("foobar", "barfoo2000", true);
	test(!strcmp(stx::env::get("foobar"), "barfoo2000"));
	stx::env::unset("foobar");
	test(!stx::env::get("foobar"));
}

static
void test_environment_replace() {
	// wo/ replacement
	test(stx::env::replace("Heyho") == "Heyho");
	test(stx::env::replace("H") == "H");
	test(stx::env::replace("") == "");
	// Multi character replacements ${HOME}

	test(
		stx::env::replace("Foo${HOME}bar") ==
		"Foo" + std::string(stx::env::get("HOME")) + "bar"
	);
	test(
		stx::env::replace("Foo${HOME}bar${HOME}power") ==
		"Foo" + std::string(stx::env::get("HOME")) + "bar" + stx::env::get("HOME") + "power"
	);
	test(
		stx::env::replace("${HOME}bar") ==
		std::string(stx::env::get("HOME")) + "bar"
	);
	test(
		stx::env::replace("Foo${HOME}") ==
		"Foo" + std::string(stx::env::get("HOME"))
	);
	test(
		stx::env::replace("${HOME}") == std::string(stx::env::get("HOME"))
	);
	test(
		stx::env::replace("${ThisDoesNotExist}") == std::string("")
	);

	// Multi character replacements $a
	stx::env::set("a", "-ducking-");
	test(
		stx::env::replace("$a") == std::string(stx::env::get("a"))
	);
	test(
		stx::env::replace("Foo$a") == "Foo" + std::string(stx::env::get("a"))
	);
	test(
		stx::env::replace("Foo$abar") == "Foo" + std::string(stx::env::get("a")) + "bar"
	);
	test(
		stx::env::replace("$abar") == std::string(stx::env::get("a")) + "bar"
	);
	test(
		stx::env::replace("Foo$abar$apower") ==
			"Foo" + std::string(stx::env::get("a")) +
			"bar" + std::string(stx::env::get("a")) + "power"
	);

	// Test all the exceptions
	try {
		stx::env::replace("Foo$");
		test(!"Didn't throw an exception");
	}
	catch(std::runtime_error& e) {
		test("Succesfully caught an exception");
	}
	try {
		stx::env::replace("Foo${");
		test(!"Didn't throw an exception");
	}
	catch(std::runtime_error& e) {
		test("Succesfully caught an exception");
	}
	try {
		stx::env::replace("Foo${bar");
		test(!"Didn't throw an exception");
	}
	catch(std::runtime_error& e) {
		test("Succesfully caught an exception");
	}
}

void test_environment() {
	test_environment_set_unset_get();
	test_environment_replace();
}

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

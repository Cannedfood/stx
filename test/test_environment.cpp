// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "catch.hpp"

#include "../environment.hpp"

TEST_CASE("Test environment set unset & get", "[environment]") {
	stx::env::set("foobar", "barfoo", false);
	CHECK(!strcmp(stx::env::get("foobar"), "barfoo"));
	stx::env::set("foobar", "barfoo2000", false);
	CHECK(strcmp(stx::env::get("foobar"), "barfoo2000"));
	stx::env::set("foobar", "barfoo2000", true);
	CHECK(!strcmp(stx::env::get("foobar"), "barfoo2000"));
	stx::env::unset("foobar");
	CHECK(!stx::env::get("foobar"));
}

TEST_CASE("Test environment replace", "[environment]") {
	// wo/ replacement
	CHECK(stx::env::replace("Heyho") == "Heyho");
	CHECK(stx::env::replace("H") == "H");
	CHECK(stx::env::replace("") == "");
	// Multi character replacements ${HOME}

	CHECK(
		stx::env::replace("Foo${HOME}bar") ==
		"Foo" + std::string(stx::env::get("HOME")) + "bar"
	);
	CHECK(
		stx::env::replace("Foo${HOME}bar${HOME}power") ==
		"Foo" + std::string(stx::env::get("HOME")) + "bar" + stx::env::get("HOME") + "power"
	);
	CHECK(
		stx::env::replace("${HOME}bar") ==
		std::string(stx::env::get("HOME")) + "bar"
	);
	CHECK(
		stx::env::replace("Foo${HOME}") ==
		"Foo" + std::string(stx::env::get("HOME"))
	);
	CHECK(
		stx::env::replace("${HOME}") == std::string(stx::env::get("HOME"))
	);
	CHECK(
		stx::env::replace("${ThisDoesNotExist}") == std::string("")
	);

	// Multi character replacements $a
	stx::env::set("a", "-ducking-");
	CHECK(
		stx::env::replace("$a") == std::string(stx::env::get("a"))
	);
	CHECK(
		stx::env::replace("Foo$a") == "Foo" + std::string(stx::env::get("a"))
	);
	CHECK(
		stx::env::replace("Foo$abar") == "Foo" + std::string(stx::env::get("a")) + "bar"
	);
	CHECK(
		stx::env::replace("$abar") == std::string(stx::env::get("a")) + "bar"
	);
	CHECK(
		stx::env::replace("Foo$abar$apower") ==
			"Foo" + std::string(stx::env::get("a")) +
			"bar" + std::string(stx::env::get("a")) + "power"
	);

	// Test all the exceptions
	CHECK_THROWS_AS(stx::env::replace("Foo$"), std::runtime_error);
	CHECK_THROWS_AS(stx::env::replace("Foo${"), std::runtime_error);
	CHECK_THROWS_AS(stx::env::replace("Foo${bar"), std::runtime_error);
}

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

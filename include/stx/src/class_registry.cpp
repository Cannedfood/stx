// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

#include "../class_registry.hpp"

#include <iostream>
#include <string>

namespace stx {

class TestBase0 {};
class TestBase1 {};
class TestImpl : public TestBase0, public TestBase1 {};

void classes::print(std::ostream& s) {
	// TODO: Print as tree structure

	for(auto& e : entries()) {
		s << e.name() << std::endl;
		if(!e.implements().empty()) {
			s << "  Implements:" << std::endl;
			for(entry& ee : e.implements()) {
				s << "\t" << ee.name() << std::endl;
			}
		}
		if(!e.implementations().empty()) {
			s << "  Implementations:" << std::endl;
			for(entry& ee : e.implementations()) {
				s << "\t" << ee.name() << std::endl;
			}
		}
	}
}

list<classes::entry>& classes::entries() noexcept {
	static list<entry> entries;
	return entries;
}

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

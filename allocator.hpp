// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

// Warning: These aren't allocators in the STL sense

#include <cstddef>
#include <utility>

namespace stx {

class arena_allocator {
public:
	arena_allocator(size_t block_size = 4096 - sizeof(char*));
	~arena_allocator();

	arena_allocator(arena_allocator&& other) noexcept;
	arena_allocator& operator=(arena_allocator&& other) noexcept;

	char* alloc(size_t bytes);
	char* alloc_string(size_t n);
	void  reset();

	void undo_alloc(size_t bytes);

	template<class T, class... Args>
	T* create(Args... args);

private:
	size_t m_arena_size;
	char*  m_arena;
	char*  m_top;
	char*  m_arena_end;
};

// TODO: implement ring_allocator

} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx {

template<class T, class... Args>
T* arena_allocator::create(Args... args) {
	try {
		return new(alloc(sizeof(T))) T(std::forward<Args>(args)...);
	}
	catch(...) {
		undo_alloc(sizeof(T));
		throw;
	}
}

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

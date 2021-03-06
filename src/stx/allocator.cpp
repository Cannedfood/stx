#include "allocator.hpp"

namespace stx {

static
char*& _old_arena(char* arena) {
	return *((char**) arena);
}

static
char* _create_arena(size_t size, char* old, char** top, char** end) {
	char* result = new char[size + sizeof(char*)];
	_old_arena(result) = old;
	if(top != nullptr)
		*top = result + sizeof(char*);
	if(end != nullptr)
		*end = *top + size;
	return result;
}

static
void _destroy_arena(char* arena) {
	while(arena) {
		char* old_arena = _old_arena(arena);
		delete[] arena;
		arena = old_arena;
	}
}

arena_allocator::arena_allocator(size_t block_size) noexcept :
	m_arena_size(block_size),
	m_arena(nullptr),
	m_top(nullptr),
	m_arena_end(nullptr)
{}

arena_allocator::~arena_allocator() noexcept {
	reset();
}

arena_allocator::arena_allocator(arena_allocator&& other) noexcept {
	m_arena = other.m_arena; other.m_arena = nullptr;
	m_top = other.m_top; other.m_top = nullptr;
	m_arena_end = other.m_arena_end; other.m_arena_end = nullptr;
	m_arena_size = other.m_arena_size;
}
arena_allocator& arena_allocator::operator=(arena_allocator&& other) noexcept {
	m_arena = other.m_arena; other.m_arena = nullptr;
	m_top = other.m_top; other.m_top = nullptr;
	m_arena_end = other.m_arena_end; other.m_arena_end = nullptr;
	m_arena_size = other.m_arena_size;
	return *this;
}

void arena_allocator::reset() noexcept {
	_destroy_arena(m_arena);
	m_arena = m_top = m_arena_end = nullptr;
}

char* arena_allocator::alloc(size_t bytes) noexcept {
	char* new_top = m_top + bytes;

	if(new_top > m_arena_end) {
		if(bytes > m_arena_size) {
			// Bigger than our usual arena size:
			//  Create a new arena just for this allocation, and append it between the current one and its predecessor
			return _old_arena(m_arena) = _create_arena(bytes, _old_arena(m_arena), nullptr, nullptr);
		}
		else {
			m_arena = _create_arena(m_arena_size, m_arena, &m_top, &m_arena_end);
			new_top = m_top + bytes;
		}
	}

	char* result = m_top;
	m_top = new_top;
	return result;
}

char* arena_allocator::alloc_string(size_t n) noexcept {
	char* result = alloc(n + 1);
	result[n] = '\0';
	return result;
}

void arena_allocator::undo_alloc(size_t bytes) noexcept {
	// TODO: arena_allocator::undo_alloc
}

} // namespace stx

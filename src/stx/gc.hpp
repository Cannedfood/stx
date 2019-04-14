#pragma once

#include <cstddef>
#include <utility>
#include <iosfwd>

namespace stx {

class garbage_collector {
public:
	static void register_object(void* p, size_t size, void(*pfnDelete)(void* p, void* user), void* user = nullptr) noexcept;

	static void create_ref(void* at, void* to) noexcept;
	static void remove_ref(void* at) noexcept;

	static void mark_and_sweep() noexcept;

	static size_t refcount(void* p) noexcept;
	static size_t outrefcount(void* p) noexcept;

	static size_t total_ref_count() noexcept;
	static size_t total_obj_count() noexcept;

	static void LEAK_ALL() noexcept;

	static void printDotGraph(std::ostream&);
	static void printDotGraph(const char* path);
};

#define ctor() printf("* %p")
#define dtor() printf("x %p")

template<class T>
class gc {
public:
	constexpr gc(std::nullptr_t = nullptr) noexcept {}
	~gc() noexcept { reset(); }

	gc(gc const& other) noexcept { *this = other; }
	gc& operator=(gc const& other) noexcept { _reset(other.m_pointer); return *this; }

	gc(gc&& other) noexcept { *this = std::move(other); }
	gc& operator=(gc&& other) noexcept {
		_reset(other.m_pointer);
		if(&other != this)
			other.reset();
		return *this;
	}

	void reset() noexcept {
		if(m_pointer) {
			garbage_collector::remove_ref(this);
			m_pointer = nullptr;
		}
	}

	void _reset(T* t) noexcept {
		reset();
		m_pointer = t;
		if(m_pointer) {
			garbage_collector::create_ref(this, m_pointer);
		}
	}

	T* get()                const noexcept { return m_pointer;    }
	T& operator*()          const noexcept { return *m_pointer;   }
	T* operator->()         const noexcept { return m_pointer;    }
	T& operator[](size_t i) const noexcept { return m_pointer[i]; }

	operator bool() const noexcept { return m_pointer != nullptr; }

	bool operator==(gc const& other) const noexcept { return m_pointer == other.m_pointer; }
	bool operator!=(gc const& other) const noexcept { return m_pointer != other.m_pointer; }
	bool operator<=(gc const& other) const noexcept { return m_pointer <= other.m_pointer; }
	bool operator>=(gc const& other) const noexcept { return m_pointer >= other.m_pointer; }
	bool operator< (gc const& other) const noexcept { return m_pointer <  other.m_pointer; }
	bool operator> (gc const& other) const noexcept { return m_pointer >  other.m_pointer; }

private:
	T* m_pointer = nullptr;
};

template<class T, class... Args>
gc<T> make_gc(Args&&... args) {
	gc<T> result;

	auto* p = new T(std::forward<Args>(args)...);
	garbage_collector::register_object(p, sizeof(T), [](void* p, void* user) { delete (T*)p; });
	result._reset(p);

	return result;
}

} // namespace stx

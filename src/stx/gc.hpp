#pragma once

#include <typeinfo>
#include <cstddef>
#include <utility> // std::move
#include <limits> // std::numeric_limits<size_t>
#include <iosfwd>

namespace stx {

template<class T> class gc;
template<class T> class weak_gc;

namespace garbage_collector {
	using Deleter = void(*)(void* obj, void* data);

	void hint_external_object(void* obj, size_t size, std::type_info const* type) noexcept;
	template<class T> void hint_external_object(T const* p, size_t n = 1) noexcept { hint_external_object((void*)p, sizeof(T) * n, n > 1 ? &typeid(T[]) : &typeid(T)); }
	void unhint_external_object(void* obj) noexcept;

	void add_object(void* obj, size_t size, Deleter del, void* del_data, std::type_info const* type) noexcept;

	void reference_removed(void* from, void* to) noexcept;
	void reference_added(void* from, void* to) noexcept;

	size_t timestamp_of(void* obj) noexcept;
	bool   is_valid(void* obj, size_t timestamp) noexcept;

	void mark_and_sweep() noexcept;

	void mark() noexcept;
	void collect() noexcept;

	size_t total_ref_count() noexcept;
	size_t total_obj_count() noexcept;

	size_t refcount(void* p) noexcept;
	size_t outrefcount(void* p) noexcept;


	void writeDotFile(std::ostream& to, bool externalReferences = true);
	void writeDotFile(std::string const& file, bool externalReferences = true);

	void LEAK_ALL() noexcept;
};

template<class T>
class gc final {
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
			garbage_collector::reference_removed(this, m_pointer);
			m_pointer = nullptr;
		}
	}

	void _reset(T* t) noexcept {
		reset();
		m_pointer = t;
		if(m_pointer) {
			garbage_collector::reference_added(this, m_pointer);
		}
	}

	template<class Tx> gc<Tx> cast_static()      const noexcept { gc<Tx> result; result._reset(static_cast<Tx*>(m_pointer)); return result; }
	template<class Tx> gc<Tx> cast_dynamic()     const noexcept { gc<Tx> result; result._reset(dynamic_cast<Tx*>(m_pointer)); return result; }
	template<class Tx> gc<Tx> cast_reinterpret() const noexcept { gc<Tx> result; result._reset(reinterpret_cast<Tx*>(m_pointer)); return result; }
	template<class Tx> operator gc<Tx>() const noexcept {
		static_assert(
			std::is_same_v<Tx, void> || // Can always cast to void*
			std::is_base_of_v<Tx, T>, // Can always count to base
			"T* is not implicitly convertable to Tx*"
		);
		return cast_static<Tx>();
	}

	T*    get()                const noexcept { return m_pointer;    }
	auto& operator*()          const noexcept { return *m_pointer;   }
	T*    operator->()         const noexcept { return m_pointer;    }
	auto& operator[](size_t i) const noexcept { return m_pointer[i]; }

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

template<class T>
gc<T> get_gc(T* t) {
	gc<T> result;
	result._reset(t);
	return result;
}

template<class T, class... Args>
gc<T> make_gc(Args&&... args) {
	auto* memory = ::operator new(sizeof(T));
	auto  result = get_gc((T*)memory);
	new(memory) T(std::forward<Args>(args)...);
	garbage_collector::add_object(
		memory, sizeof(T),
		[](void* p, void* user) {
			((T*)p)->~T();
			::operator delete(p);
		}, nullptr,
		&typeid(T)
	);
	return result;
}

template<class T>
class weak_gc final {
public:
	constexpr weak_gc(std::nullptr_t = nullptr) noexcept {}

	template<class Tx>
	weak_gc(gc<Tx> const& t) noexcept { *this = t; }

	template<class Tx> constexpr
	weak_gc& operator=(gc<Tx> const& t) noexcept {
		m_pointer   = t.get();
		m_timestamp = garbage_collector::timestamp_of(t.get());
		return *this;
	}

	gc<T> lock() const noexcept {
		gc<T> result;
		if(garbage_collector::is_valid(m_pointer, m_timestamp)) {
			result._reset(m_pointer);
		}
		return result;
	}
private:
	T*      m_pointer = nullptr;
	size_t  m_timestamp = 0;
};

template<class T = char>
class gc_alloc {
public:
	using value_type      = T;
	using pointer         = T*;
	using const_pointer   = const T*;
	using reference       = T&;
	using const_reference = const T&;
	using size_type       = std::size_t;
	using difference_type = std::ptrdiff_t;

	template<class U> struct rebind { using other = gc_alloc<U>; };

	pointer       address(reference value) const noexcept { return &value; }
	const_pointer address(const_reference value) const noexcept { return &value; }

	gc_alloc(void* owner) noexcept : m_owner(owner) {}
	template <class U>
	gc_alloc(gc_alloc<U> const& other) noexcept : m_owner(other.owner()) {}
	~gc_alloc() noexcept {}

	// return maximum number of elements that can be allocated
	size_type max_size() const noexcept {
		return std::numeric_limits<std::size_t>::max() / sizeof(T);
	}

	// allocate but don't initialize num elements of type T
	pointer allocate(size_type num, const void* = 0) const noexcept {
		size_t size = num * sizeof(T);
		pointer new_memory = (pointer)(::operator new(size));

		garbage_collector::add_object(
			new_memory, size,
			[](void* p, void* user) { ::operator delete(p); }, nullptr,
			num > 1 ? &typeid(T[]) : &typeid(T)
		);
		garbage_collector::reference_added(m_owner, new_memory);

		return new_memory;
	}

	void deallocate(pointer p, size_type num) const noexcept {
		garbage_collector::reference_removed(m_owner, p);
	}

	void construct(pointer p, const T& value) const { new((void*)p)T(value); }
	void destroy  (pointer p) const { p->~T(); }

	void* owner() const noexcept { return m_owner; }
private:
	void* m_owner;
};

} // namespace stx

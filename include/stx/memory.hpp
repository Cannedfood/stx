#pragma once

#include <cstddef>

#include <utility>

#include "type.hpp"

namespace stx {

class refcounted_interface;

template<typename T>
class weak;

template<typename T, typename Tdel>
class owned;


// == owned<T, Tdelete> ==============================================================

template<typename T, typename Tdel = stx::default_delete<T>>
class owned {
private:
	using Tself = owned<T, Tdel>;

	mutable T*   m_pointer;
	mutable Tdel m_deleter;

public:
	constexpr inline
	owned(T* ptr = nullptr, const Tdel& del = Tdel()) :
		m_pointer(ptr),
		m_deleter(del)
	{}

	inline
	~owned() {
		reset();
	}

	constexpr inline
	owned(Tself&& other) :
		m_pointer(std::move(other.m_pointer)),
		m_deleter(std::move(other.m_deleter))
	{
		other.m_pointer = nullptr;
	}

	constexpr inline
	Tself& operator=(Tself&& other) {
		reset(other.m_pointer, other.m_deleter);
		other.m_pointer = nullptr;
		return *this;
	}

	owned(Tself const&)            = delete;
	Tself& operator=(Tself const&) = delete;

	constexpr inline
	void reset(T* ptr = nullptr, const Tdel& del = Tdel()) {
		if(m_pointer) {
			m_deleter(m_pointer);
		}
		m_pointer = nullptr;
		m_deleter = del;
	}

	constexpr inline
	operator bool() const {
		return m_pointer != nullptr;
	}

	constexpr inline
	T* get() const { return m_pointer; }

	T* operator->() const { return m_pointer; }
	T& operator*()  const { return *m_pointer; }
};


// == shared<T> ==============================================================

namespace detail {

class shared_block {
private:
	unsigned m_shared_refs;
	unsigned m_weak_refs;

protected:

	constexpr inline
	shared_block() :
		m_shared_refs(0),
		m_weak_refs(0)
	{}

	inline
	void decrement_shared() noexcept {
		--m_shared_refs;
		if(m_shared_refs == 0) {
			destroy_pointer();
		}
	}

	inline
	void increment_shared() noexcept {
		++m_shared_refs;
	}

	inline
	void decrement_weak() noexcept {
		--m_weak_refs;
		if(m_weak_refs == 0) {
			destroy_block();
		}
	}

	inline
	void increment_weak() noexcept {
		++m_weak_refs;
	}

	virtual void destroy_pointer() = 0;
	virtual void destroy_block() = 0;

	struct shared_ownership {
		constexpr inline
		void operator()(shared_block* b) const noexcept {
			b->decrement_shared();
		}
	};

	struct weak_ownership {
		constexpr inline
		void operator()(shared_block* b) const noexcept {
			b->decrement_weak();
		}
	};

public:
	using shared_ref = owned<shared_block, shared_ownership>;
	using weak_ref = owned<shared_block, weak_ownership>;

	inline int shared_refs() const noexcept { return m_shared_refs; }
	inline int weak_refs()   const noexcept { return m_weak_refs; }

	inline
	shared_ref new_shared_ref() noexcept {
		increment_shared();
		return shared_ref(this);
	}

	inline
	weak_ref   new_weak_ref() noexcept {
		increment_weak();
		return weak_ref(this);
	}

	virtual ~shared_block() {}
};

template<typename T, typename Tdel = stx::default_delete<T>>
class simple_shared_block : public shared_block, owned<T, Tdel> {
	T* m_pointer;

public:
	T* pointer() noexcept { return m_pointer; }

};

} // namespace detail

template<typename T>
class shared {
	detail::shared_block::shared_ref m_shared_block;
	T*                               m_pointer;

public:
	shared()  {}
	~shared() {}

	T* get() const { return m_pointer; }
};


// == weak<T> ==============================================================

template<typename T>
class weak {
	using Tself = weak<T>;

	mutable detail::shared_block::weak_ref m_shared_block;
	mutable T* m_pointer;

public:
	weak(detail::shared_block* block, T* ptr) :
		m_shared_block(block ? block->new_weak_ref() : nullptr),
		m_pointer(ptr)
	{}

	weak(const Tself& other) {
		if(other.m_shared_block && other.m_shared_block->shared_refs())
			m_shared_block = other.m_shared_block->new_shared_ref();
		m_pointer = other.m_pointer;
	}

	weak(Tself&& other) :
		m_shared_block(std::move(other.m_shared_block)),
		m_pointer(other)
	{}

	void operator=(const Tself& other) {

	}

	~weak() {}

	operator bool() const {
		if(m_shared_block && m_shared_block->shared_refs())
			return true;
		else {
			m_shared_block.reset();
			m_pointer = nullptr;
			return false;
		}
	}

	shared<T> lock() noexcept {
		if(*this)
			return shared<T>(m_shared_block.get(), m_pointer);
		else
			return nullptr;
	}

	T* get() const noexcept {
		if(*this)
			return m_pointer;
		else
			return nullptr;
	}

	T* operator->() const { return get(); }
	T& operator*()  const { return *get(); }
};

} // namespace stx

#pragma once

#include <cstddef>

#include <utility>

#include "type.hpp"

#ifdef STX_DEBUG
#	include "assert.hpp"
#endif

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
	using Tptr  = pointer_to<T>;
	using Tref  = remove_pointer<Tptr>;

	mutable Tptr m_pointer;
	mutable Tdel m_deleter;

public:
	constexpr inline
	owned(Tptr ptr = nullptr, const Tdel& del = Tdel()) :
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
	void reset(Tptr ptr = nullptr, const Tdel& del = Tdel()) {
		if(m_pointer) {
			m_deleter(m_pointer);
		}
		m_pointer = nullptr;
		m_deleter = del;
	}

	constexpr inline
	Tptr release() noexcept {
		auto tmp = std::move(m_pointer);
		m_pointer = nullptr;
		return tmp;
	}

	constexpr inline
	operator bool() const noexcept {
		return m_pointer != nullptr;
	}

	constexpr inline
	T* get() const noexcept { return m_pointer; }

	constexpr inline
	T* operator->() const noexcept { return m_pointer; }

	constexpr inline
	T& operator*()  const noexcept { return *m_pointer; }
};

template<typename T, typename Tdel = stx::default_delete<T>> constexpr inline
owned<T, Tdel> own(T* t, Tdel const& del = Tdel()) noexcept { return owned<T, Tdel>(t, del); }

template<typename T, typename... ARGS>
owned<T> new_owned(ARGS&&... args) noexcept { return owned<T>(new T(std::forward<ARGS>(args)...)); }


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
		inline
		void operator()(shared_block* b) const noexcept {
			b->decrement_shared();
		}
	};

	struct weak_ownership {
		inline
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
class simple_shared_block : public shared_block {
	using Tptr = pointer_to<T>;

	Tptr m_pointer;
	Tdel m_deleter;

public:
	constexpr
	simple_shared_block(Tptr ptr, const Tdel& del = Tdel()) :
		m_pointer(ptr),
		m_deleter(m_deleter)
	{}

	inline Tptr pointer() noexcept { return m_pointer; }

	void destroy_pointer() override {
		m_deleter(m_pointer);
	}

	void destroy_block()   override {
		delete this;
	}
};

template<typename T>
class aligned_shared_block : public shared_block {
	char m_data[sizeof(T)];

public:
	template<typename... ARGS> constexpr
	aligned_shared_block(ARGS&&... args) {
#ifdef STX_DEBUG
		xassertmsg(
			new (m_data) T(std::forward<ARGS>(args)...) == pointer(),
			"STX code error! Please open an issue with reproduction information on github!"
		);
#else
		new (m_data) T(std::forward<ARGS>(args)...);
#endif
	}

	~aligned_shared_block() {
		reinterpret_cast<T*>(m_data)->~T();
	}

	inline T* pointer() noexcept { return reinterpret_cast<T*>(m_data); }

	void destroy_pointer() override {
		pointer()->~T();
	}

	void destroy_block()   override {
		delete this;
	}
};

} // namespace detail

template<typename T>
class shared {
	using Tself = shared<T>;
	using Tptr  = pointer_to<T>;
	using Tref  = remove_pointer<Tptr>;

	detail::shared_block::shared_ref m_shared_block;
	mutable Tptr                     m_pointer;

public:
	constexpr
	shared() :
		m_pointer(nullptr)
	{}

	~shared() {}

	shared(Tself const& other) :
		m_shared_block(other.m_shared_block ? m_shared_block->new_shared_ref() : nullptr),
		m_pointer(other.m_pointer)
	{}

	shared(Tself&& other) :
		m_shared_block(std::move(other.m_shared_block)),
		m_pointer(other.m_pointer)
	{
		other.m_pointer = nullptr;
	}

	Tself& operator=(Tself const& other) noexcept {
		m_shared_block  = other.m_shared_block ? other.m_shared_block->new_shared_ref() : nullptr;
		m_pointer       = other.m_pointer;
		other.m_pointer = nullptr;
		return *this;
	}

	Tself& operator=(Tself&& other) noexcept {
		m_shared_block  = std::move(other.m_shared_block);
		m_pointer       = other.m_pointer;
		other.m_pointer = nullptr;
		return *this;
	}

	inline Tptr get()        const noexcept { return m_pointer; }
	inline Tptr operator->() const noexcept { return m_pointer; }
	inline Tref operator*()  const noexcept { return *m_pointer; }

	template<typename Idx, typename = typename std::enable_if<is_array<T>>>
	Tref operator[](Idx i) const noexcept {
		return m_pointer[i];
	}
};

template<typename T, typename Tdel = stx::default_delete<T>>
owned<T, Tdel> share(T* t, Tdel const& del = Tdel()) noexcept {
	return share<T>(new detail::simple_shared_block<T, Tdel>(t, del), t);
}

template<typename T, typename... ARGS>
owned<T> new_shared(ARGS&&... args) noexcept {
	auto* block = new detail::aligned_shared_block<T>(std::forward<ARGS>(args)...);
	return share<T>(block, block->pointer());
}


// == weak<T> ==============================================================

template<typename T>
class weak {
	using Tself = weak<T>;
	using Tshared = shared<T>;

	mutable detail::shared_block::weak_ref m_shared_block;
	mutable T* m_pointer;

public:
	weak(detail::shared_block* block, T* ptr) :
		m_shared_block(),
		m_pointer(nullptr)
	{
		if(block && block->shared_refs()) {
			m_shared_block = block->new_weak_ref();
			m_pointer      = ptr;
		}
		else {
			m_shared_block = nullptr;
			m_pointer      = nullptr;
		}
	}

	void reset(detail::shared_block* block, T* ptr) {
		if(block && block->shared_refs()) {
			m_shared_block = block->new_weak_ref();
			m_pointer      = ptr;
		}
		else {
			m_shared_block = nullptr;
			m_pointer      = nullptr;
		}
	}

	void reset() {
		m_shared_block = nullptr;
		m_pointer      = nullptr;
	}

	weak(Tself const& other) :
		weak(other.m_shared_block.get(), other.pointer)
	{
		if(other.m_shared_block && other.m_shared_block->shared_refs())
			m_shared_block = other.m_shared_block->new_weak_ref();
		m_pointer = other.m_pointer;
	}

	weak(Tself&& other) :
		m_shared_block(std::move(other.m_shared_block)),
		m_pointer(other)
	{}

	weak(Tshared const& s) :
		weak()
	{}

	void operator=(Tself const& other) {
		reset(other.m_shared_block.get(), other.m_pointer);
	}

	void operator=(Tself&& other) {
		reset(other.m_shared_block.release(), other.m_pointer);
		other.m_pointer = nullptr;
	}

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

	inline
	T* get() const noexcept {
		if(*this)
			return m_pointer;
		else
			return nullptr;
	}

	inline T* operator->() const noexcept { return get(); }
	inline T& operator*()  const noexcept { return *get(); }
};

} // namespace stx

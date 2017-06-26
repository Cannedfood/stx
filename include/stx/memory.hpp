#pragma once

#include <cstddef>

#include <utility>

#include "type.hpp"

#ifdef STX_DEBUG
#	include "assert.hpp"
#endif

namespace stx {

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
	using Tref  = remove_pointer<Tptr>&;

	mutable Tptr m_pointer;
	mutable Tdel m_deleter;

public:
	constexpr inline
	owned(std::nullptr_t = nullptr) :
		m_pointer(nullptr),
		m_deleter()
	{}

	constexpr inline
	owned(Tptr ptr, const Tdel& del = Tdel()) :
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
		m_pointer = ptr;
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
	Tptr get() const noexcept { return m_pointer; }

	constexpr inline
	Tptr operator->() const noexcept { return m_pointer; }

	constexpr inline
	Tref operator*()  const noexcept { return *m_pointer; }

	constexpr inline
	operator Tptr() const noexcept { return m_pointer; }

	template<typename Idx, typename = typename std::enable_if<is_array<T>>>
	Tref operator[](Idx i) const noexcept {
		return m_pointer[i];
	}
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

			if(m_weak_refs == 0) {
				destroy_block();
			}
		}
	}

	inline
	void increment_shared() noexcept {
		++m_shared_refs;
	}

	inline
	void decrement_weak() noexcept {
		--m_weak_refs;
		if(m_weak_refs == 0 && m_shared_refs == 0) {
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
	using weak_ref   = owned<shared_block, weak_ownership>;

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

	owned<T, Tdel> m_pointer;

public:
	constexpr
	simple_shared_block(Tptr ptr, const Tdel& del = Tdel()) :
		m_pointer(ptr, del)
	{}

	constexpr
	simple_shared_block(owned<T, Tdel>&& p) :
		m_pointer(std::move(p))
	{}

	inline Tptr pointer() noexcept { return m_pointer.get(); }

	void destroy_pointer() override {
		m_pointer.reset();
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
		T* p = new (m_data) T(std::forward<ARGS>(args)...);
		xassertmsg(
			p == pointer(),
			"STX code error! Please open an issue with reproduction information on github!"
		);
#else
		new (m_data) T(std::forward<ARGS>(args)...);
#endif
	}

	~aligned_shared_block() {}

	inline T* pointer() noexcept { return reinterpret_cast<T*>(m_data); }

	void destroy_pointer() override {
		pointer()->~T();
	}

	void destroy_block()   override {
		delete this;
	}
};

template<typename T, typename Callback>
class aligned_shared_block_with_callback : public aligned_shared_block<T> {
	Callback mCallback;

	template<typename... ARGS>
	aligned_shared_block_with_callback(Callback&& c, ARGS&&... args) :
		aligned_shared_block<T>(std::forward<ARGS>(args)...),
		mCallback(c)
	{}

	void destroy_pointer() override {
		mCallback();
	}
};

} // namespace detail

template<typename T>
class shared {
	using Tself = shared<T>;
	using Tptr  = pointer_to<T>;
	using Tref  = remove_pointer<Tptr>&;

	mutable detail::shared_block::shared_ref m_shared_block;
	mutable Tptr                             m_pointer;

public:
	constexpr
	shared(std::nullptr_t = nullptr) :
		m_shared_block(nullptr),
		m_pointer(nullptr)
	{}

	shared(detail::shared_block* block, Tptr ptr) :
		m_shared_block(block ? block->new_shared_ref() : nullptr),
		m_pointer(block ? ptr : nullptr)
	{}

	shared(Tself const& other) :
		shared(other.m_shared_block.get(), other.m_pointer)
	{}

	shared(Tself&& other) :
		m_shared_block(std::move(other.m_shared_block)),
		m_pointer(other.m_pointer)
	{
		other.m_pointer = nullptr;
	}

	template<typename Tdel>
	shared(owned<T, Tdel>&& t) {
		reset(std::move(t));
	}

	void reset(std::nullptr_t = nullptr) {
		m_shared_block.reset();
		m_pointer = nullptr;
	}

	void reset(detail::shared_block* block, Tptr ptr) {
		if(block && block->shared_refs() != 0) {
			m_shared_block = block->new_shared_ref();
			m_pointer      = ptr;
		}
	}

	template<typename Tdel>
	void reset(owned<T, Tdel>&& t) {
		if(t) {
			m_pointer      = t.get();
			m_shared_block = (new detail::simple_shared_block<T, Tdel>(std::move(t)))->new_shared_ref();
		}
	}

	template<typename Tdel>
	void reset(Tptr p, Tdel const& del = Tdel()) {
		if(p) {
			m_pointer      = p;
			m_shared_block = (new detail::simple_shared_block<T, Tdel>(p, del))->new_shared_ref();
		}
	}

	template<typename Tx>
	shared<Tx> cast() {
		return shared<Tx>(m_shared_block.get(), static_cast<pointer_to<Tx>>(m_pointer));
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

	template<typename Tdel>
	Tself& operator=(owned<T, Tdel>&& p) noexcept {
		reset(std::move(p));
		return *this;
	}

	constexpr inline Tptr get()        const noexcept { return m_pointer; }
	constexpr inline Tptr operator->() const noexcept { return m_pointer; }
	constexpr inline Tref operator*()  const noexcept { return *m_pointer; }

	constexpr inline detail::shared_block* get_block() const noexcept { return m_shared_block.get(); }

	constexpr inline
	operator Tptr() const noexcept { return m_pointer; }

	constexpr inline
	operator bool() const noexcept { return m_pointer != nullptr; }

	template<typename Idx, typename = typename std::enable_if<is_array<T>, Idx>::type>
	Tref operator[](Idx i) const noexcept {
		return m_pointer[i];
	}
};

template<typename T, typename Tdel = stx::default_delete<T>>
shared<T> share(T* t, Tdel const& del = Tdel()) noexcept {
	return shared<T>(new detail::simple_shared_block<T, Tdel>(t, del), t);
}

template<typename T, typename Tdel = stx::default_delete<T>, typename C>
shared<T> share_with_callback(T* t, C&& c, Tdel const& del = Tdel()) {
	auto actual_deleter = [c, del](T* t) {
		c(t);
		del(t);
	};
	using Tactual_deleter = typename std::remove_reference<decltype(actual_deleter)>::type;

	auto* block = new detail::simple_shared_block<T, Tactual_deleter>(t, std::move(actual_deleter));

	return shared<T>((detail::shared_block*) block, block->pointer());
}

template<typename T, typename... ARGS>
shared<T> new_shared(ARGS&&... args) noexcept {
	auto* block = new detail::aligned_shared_block<T>(std::forward<ARGS>(args)...);
	return shared<T>((detail::shared_block*) block, block->pointer());
}

template<typename T, typename C, typename... ARGS>
shared<T> new_shared_with_callback(C&& c, ARGS&&... args) noexcept {
	using callback_type = typename std::remove_reference<C>::type;
	auto* block = new detail::aligned_shared_block_with_callback<T, callback_type>(c, std::forward<ARGS>(args)...);
	return shared<T>((detail::shared_block*) block, block->pointer());
}

// == weak<T> ========================================================

template<typename T>
class weak {
	using Tself = weak<T>;
	using Tshared = shared<T>;

	mutable detail::shared_block::weak_ref m_shared_block;
	mutable T* m_pointer;

public:
	// -- Constructor -----------------------------------------

	weak() :
		m_shared_block(),
		m_pointer(nullptr)
	{}

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

	// -- Copy & Move ---------------------------------------------
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

	Tself& operator=(Tself const& other) {
		reset(other.m_shared_block.get(), other.m_pointer);
		return *this;
	}

	Tself& operator=(Tself&& other) {
		reset(other.m_shared_block.release(), other.m_pointer);
		other.m_pointer = nullptr;
		return *this;
	}

	// -- Assignment --------------------------------------------
	weak(Tshared const& s) :
		weak(s.get_block(), s.get())
	{}

	Tself& operator=(Tshared const& s) {
		reset(s);
		return *this;
	}

	// -- General operations -----------------------------------

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

	void reset(Tshared const& s) {
		reset(s.get_block(), s.get());
	}

	void reset() {
		m_shared_block = nullptr;
		m_pointer      = nullptr;
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

	operator bool() const noexcept {
		if(m_shared_block && m_shared_block->shared_refs())
			return true;
		else {
			m_shared_block.reset();
			m_pointer = nullptr;
			return false;
		}
	}

	inline T* operator->() const noexcept { return get(); }
	inline T& operator*()  const noexcept { return *get(); }
};

} // namespace stx

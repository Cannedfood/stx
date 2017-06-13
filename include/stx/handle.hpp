#pragma once

#include <utility>
#include <vector>

#ifdef STX_DEBUG
#	include "assert.hpp"
#endif

namespace stx {

template<typename T, typename Tother = T>
class pointer_pair {
	using Tself  = pointer_pair<T, Tother>;

	Tother* m_other;

	friend class pointer_pair<Tother, T>;
public:
	// -- Constructor ---------------------------------------------------------
	pointer_pair() : m_other(nullptr) {}
	pointer_pair(Tother* t) : m_other(t) {}
	~pointer_pair() noexcept {
		reset();
	}

	// -- Copy --------------------------------------------------------------
	pointer_pair(Tself const&)     = delete;
	Tself& operator=(Tself const&) = delete;

	// -- Move --------------------------------------------------------------
	pointer_pair(Tself&& other) :
		pointer_pair()
	{
		*this = std::move(other);
	}
	Tself& operator=(Tself&& other) noexcept {
		reset(other.m_other);
		return *this;
	}

	// -- Functionality ----------------------------------------------------
	void reset(Tother* other = nullptr) noexcept {
		if(m_other) {
#ifdef STX_DEBUG
			xassert(m_other->m_other == this);
#endif
			m_other->m_other = nullptr;
		}

		m_other = other;
		if(m_other) {
			m_other->reset();
			m_other->m_other = (T*) this;
		}
	}

	Tother* get() noexcept { return m_other; }
};

class handle;

class handle_socket : public pointer_pair<handle_socket, handle> {
protected:
	virtual void on_handle_destroyed() noexcept = 0;

	friend class handle;
};

class handle : public pointer_pair<handle, handle_socket> {
public:
	handle() {}

	handle(handle_socket* h) :
		pointer_pair<handle, handle_socket>(h)
	{}

	~handle() noexcept {
		reset();
	}

	void reset(handle_socket* h = nullptr) noexcept {
		if(get()) {
			get()->on_handle_destroyed();
		}
		pointer_pair<handle, handle_socket>::reset(h);
	}
};

} // namespace stx

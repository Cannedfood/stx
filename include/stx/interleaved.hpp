#pragma once

#include <cstdint>

namespace stx {

/// An interleaved array of something
template<typename T>
class interleaved {
	using size_t = std::size_t;

	T*     m_first;
	size_t m_byte_stride;

public:
	using self_t        = interleaved<T>;
	using value_t       = T;
	using const_value_t = T const;
	using ref_t         = T&;

	constexpr
	interleaved(std::nullptr_t = nullptr) :
		m_first(nullptr),
		m_byte_stride(0)
	{}

	constexpr
	interleaved(T* first, size_t byte_stride = sizeof(T)) :
		m_first(first),
		m_byte_stride(byte_stride)
	{}

	constexpr
	interleaved(void* first, size_t byte_stride = sizeof(T), size_t byte_offset = 0) :
		m_first(reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(first) + byte_offset)),
		m_byte_stride(byte_stride)
	{}

	void reset(T* first, size_t byte_stride) noexcept {
		m_first = first;
		m_byte_stride = byte_stride;
	}

	void reset() noexcept { reset(nullptr, 0); }

	self_t operator+(size_t n) const { return self_t(m_first + n, m_byte_stride); }
	self_t operator-(size_t n) const { return self_t(m_first - n, m_byte_stride); }

	ref_t operator[](size_t idx) const {
		return *reinterpret_cast<T*>(
			(uint8_t*)(m_first) + m_byte_stride * idx
		);
	}

	constexpr
	operator bool() const noexcept {
		return m_first != nullptr;
	}

	constexpr
	operator interleaved<const_value_t>() const noexcept {
		return interleaved<const_value_t>(m_first, m_byte_stride);
	}
};

} // namespace stx

#pragma once

#include <cstdint>

namespace stx {

/// A handle to an entity
class entity {
	uint64_t m_id;

public:
	constexpr entity() : m_id(~0ull) {}
	constexpr entity(uint64_t id) : m_id(id) {}
	constexpr entity(uint32_t index, uint32_t version) :
		m_id(uint64_t(index) | uint64_t(version) << 32ull)
	{}

	constexpr entity(entity const&) = default;
	constexpr entity& operator=(entity const&) = default;

	constexpr uint64_t id()      const { return m_id; }
	constexpr uint32_t index()   const { return m_id & 0xFFFFFFFF; }
	constexpr uint32_t version() const { return m_id >> 32; }

	constexpr operator bool() const noexcept { return *this != entity(); }

	constexpr bool operator==(entity const& other) const noexcept { return id() == other.id(); }
	constexpr bool operator!=(entity const& other) const noexcept { return id() != other.id(); }
	constexpr bool operator<=(entity const& other) const noexcept { return id() <= other.id(); }
	constexpr bool operator>=(entity const& other) const noexcept { return id() >= other.id(); }
	constexpr bool operator< (entity const& other) const noexcept { return id() <  other.id(); }
	constexpr bool operator> (entity const& other) const noexcept { return id() >  other.id(); }
};

} // namespace stx
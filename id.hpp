#pragma once

#include <array>
#include <vector>
#include <deque>
#include <cstdint>

namespace stx {

union handle {
	struct {
		uint32_t index;
		uint32_t version;
	};
	uint64_t id;
};

template<class Managed>
class handle_provider {
	struct Entry {
		uint32_t version;
		alignas(Managed) std::array<uint8_t, sizeof(Managed)> memory;
		Managed* get() noexcept { return memory; }
	};
	std::vector<std::vector<Entry>> m_storage;
	std::deque<uint32_t>            m_free_indices;
	uint32_t                        m_free_after;
public:
	template<class... Args>
	handle   create(Args&&... args);
	void     free(handle h);
	Managed& resolve(handle h);
};

} // namespace stx

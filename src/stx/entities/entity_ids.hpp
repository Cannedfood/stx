#pragma once

#include "entity.hpp"

#include <vector>

namespace stx::ecs {

class entity_ids {
public:
	entity create() noexcept;
	void   free(entity) noexcept;
	bool   valid(entity) const noexcept;

	entity entityAt(uint32_t index) const noexcept;
private:
	std::vector<uint32_t> m_versions;
	std::vector<uint32_t> m_free;
};

} // namespace stx::ecs
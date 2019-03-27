#include "entity_ids.hpp"

#include <cassert>

namespace stx::ecs {

entity entity_ids::create() noexcept {
	if(!m_free.empty()) {
		uint32_t index = m_free.back();
		m_free.pop_back();
		return entityAt(index);
	}
	else {
		m_versions.push_back(0);
		return entityAt(m_versions.size() - 1);
	}
}

void entity_ids::free(entity e) noexcept {
	assert(e.index() < m_versions.size());
	assert(m_versions[e.index()] == e.version() && "Freeing invalid id or double free!");
	m_versions[e.index()]++;
	m_free.push_back(e.index());
}

bool entity_ids::valid(entity id) const noexcept {
	return m_versions.size() > id.index() && m_versions[id.index()] == id.version();
}

entity entity_ids::entityAt(uint32_t index) const noexcept {
	assert(index < m_versions.size());
	return {index, m_versions[index]};
}

} // namespace stx::ecs

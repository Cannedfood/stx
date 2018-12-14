#include "../entities.hpp"

#include <cassert>

#include <atomic>
#include <cstdio>

namespace stx {

// -- component id -------------------------------------------------------

size_t detail::ecs::new_component_id() noexcept {
	static std::atomic<size_t> value = 0;
	size_t result = value++;
	if(result > options::MaxNumComponents) {
		fprintf(stderr, "Exceeded MaxNumComponents (= %zu)\n", options::MaxNumComponents);
		std::terminate();
	}
	return result;
}

// -- id_manager -------------------------------------------------------
namespace detail::ecs {

entity id_manager::create() noexcept {
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

void id_manager::free(entity e) noexcept {
	assert(e.index() < m_versions.size());
	assert(m_versions[e.index()] == e.version() && "Freeing invalid id or double free!");
	m_versions[e.index()]++;
	m_free.push_back(e.index());
}

bool id_manager::valid(entity id) const noexcept {
	return m_versions.size() > id.index() && m_versions[id.index()] == id.version();
}

entity id_manager::entityAt(uint32_t index) const noexcept {
	assert(index < m_versions.size());
	return {index, m_versions[index]};
}

} // namespace detail

// -- entities -------------------------------------------------------
entities::entities() {}
entities::~entities() {
	for(unsigned entityIndex = 0; entityIndex < m_component_masks.size(); entityIndex++) {
		auto& mask = m_component_masks[entityIndex];
		if(mask.any()) {
			for(size_t componentId = 0; componentId < mask.size(); componentId++) {
				if(mask[componentId]) {
					m_component_storage[componentId]->destroyTypeErased(entityIndex);
				}
			}
		}
	}
}

entity entities::create() noexcept {
	entity result = m_ids.create();
	m_component_masks.resize(std::max<size_t>(
		m_component_masks.size(),
		result.index() + 1
	));
	return result;
}

entity entities::create(component_mask hint) noexcept {
	return create();
}

void entities::destroy(entity e) {
	m_ids.free(e);
	auto& mask = m_component_masks[e.index()];
	if(mask.any()) {
		for(size_t componentId = 0; componentId < mask.size(); componentId++) {
			if(mask[componentId]) {
				m_component_storage[componentId]->destroyTypeErased(e.index());
			}
		}
		mask.reset();
	}
}

entity entities::first(component_mask mask) {
	for(
		uint32_t entityIndex = 0;
		entityIndex < m_component_masks.size();
		entityIndex++)
	{
		if((m_component_masks[entityIndex] & mask) == mask) {
			return m_ids.entityAt(entityIndex);
		}
	}
	return entity();
}
entity entities::next(entity e, component_mask mask) {
	for(
		uint32_t entityIndex = e.index() + 1;
		entityIndex < m_component_masks.size();
		entityIndex++)
	{
		if((m_component_masks[entityIndex] & mask) == mask) {
			return m_ids.entityAt(entityIndex);
		}
	}
	return entity();
}

} // namespace stx

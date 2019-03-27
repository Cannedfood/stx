#include "entities.hpp"

#include <cassert>

#include <atomic>
#include <cstdio>

namespace stx {

// -- component id -------------------------------------------------------

std::atomic<size_t> NumRegisteredComponents = 0;

size_t ecs::new_component_id(std::type_info const& t) noexcept {
	size_t result = NumRegisteredComponents++;
	// printf("%s -> %zu\n", t.name(), result);
	if(result >= options::MaxNumComponents) {
		fprintf(stderr, "Exceeded MaxNumComponents (= %zu)\n", options::MaxNumComponents);
		std::terminate();
	}
	return result;
}

// -- entities -------------------------------------------------------
entities::entities() noexcept {}
entities::~entities() noexcept {
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

bool entities::destroy(entity e) noexcept {
	if(!valid(e)) return false;

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

	return true;
}

void* entities::getUnchecked(entity e, size_t component_id) noexcept {
	return m_component_storage[component_id]->getTypeErased(e.index());
}
void* entities::get(entity e, size_t component_id) noexcept {
	if(!m_ids.valid(e) || !m_component_masks[e.index()].test(component_id))
		return nullptr;

	return getUnchecked(e, component_id);
}
void entities::remove(entity e, size_t component_id) noexcept {
	if(m_ids.valid(e) && m_component_masks[e.index()].test(component_id)) {
		m_component_storage[component_id]->destroyTypeErased(e.index());
		m_component_masks[e.index()].reset(component_id);
	}
}


entity entities::first(component_mask mask) noexcept {
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
entity entities::next(entity e, component_mask mask) noexcept {
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

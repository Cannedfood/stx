#pragma once

#include "components.hpp"

namespace stx::ecs {

class entities;
class entity;

class entity_listener {
public:
	virtual void onCreate (entities& ecs, entity e) = 0;
	virtual void onDestroy(entities& ecs, entity e) = 0;

	entity_listener(component_mask mask) noexcept :
		m_mask(mask),
		m_entities(nullptr)
	{}

	~entity_listener() noexcept;
	//  {
	// 	if(m_entities) {
	// 		m_entities->removeListener(this);
	// 	}
	// }
private:
	const component_mask m_mask;
	entities*            m_entities;
};

} // namespace stx::ecs
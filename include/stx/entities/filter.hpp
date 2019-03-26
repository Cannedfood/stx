#pragma once

#include "entities.hpp"

namespace stx {

// ====================================================================
// == Component Filtering =============================================
// ====================================================================

class filter_id_t {
	entities&      m_entities;
	component_mask m_mask;
public:
	filter_id_t(entities& entities, component_mask mask) :
		m_entities(entities),
		m_mask(mask)
	{}

	struct iterator {
	protected:
		mutable entities* m_entities;
		entity            m_entity;
		component_mask    m_mask;
	public:
		constexpr iterator() noexcept
			: m_entities(nullptr), m_entity(), m_mask()
		{}

		constexpr iterator(entities* ecs, entity e, component_mask mask) noexcept
			: m_entities(ecs), m_entity(e), m_mask(mask)
		{}

		iterator& operator++(void) noexcept {
			m_entity = m_entities->next(m_entity, m_mask);
			return *this;
		}

		stx::entity operator*() const noexcept {
			return m_entity;
		}

		constexpr bool operator==(iterator const& other) const noexcept {
			return m_entity == other.m_entity;
		}
		constexpr bool operator!=(iterator const& other) const noexcept {
			return m_entity != other.m_entity;
		}
	};

	iterator begin() const noexcept {
		return iterator(&m_entities, m_entities.first(m_mask), m_mask);
	}
	constexpr iterator end() const noexcept {
		return iterator();
	}
};

template<class... Components>
class filter_t {
	entities&      m_entities;
	component_mask m_mask;
public:
	filter_t(entities& ecs) noexcept :
		m_entities(ecs),
		m_mask(make_component_mask<Components...>())
	{}

	struct iterator {
		mutable entities* m_entities;
		entity            m_entity;
		component_mask    m_mask;
	public:
		constexpr iterator() noexcept
			: m_entities(nullptr), m_entity(), m_mask()
		{}

		constexpr iterator(entities* ecs, entity e, component_mask mask) noexcept
			: m_entities(ecs), m_entity(e), m_mask(mask)
		{}

		iterator& operator++(void) noexcept {
			m_entity = m_entities->next(m_entity, m_mask);
			return *this;
		}

		std::tuple<Components&...> operator*() const noexcept {
			return { m_entities->getUnchecked<Components>(m_entity)... };
		}

		constexpr bool operator==(iterator const& other) const noexcept {
			return m_entity == other.m_entity;
		}
		constexpr bool operator!=(iterator const& other) const noexcept {
			return m_entity != other.m_entity;
		}
	};

	iterator begin() const noexcept {
		return iterator(&m_entities, m_entities.first(m_mask), m_mask);
	}
	constexpr iterator end() const noexcept {
		return iterator();
	}
};

} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx {

template<class... Types>
filter_t<Types...> entities::filter() noexcept {
	return {*this};
}

template<class... Types>
filter_id_t entities::filter_id() noexcept {
	return {*this, make_component_mask<Types...>()};
}

} // namespace stx